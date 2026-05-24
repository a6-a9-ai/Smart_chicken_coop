#include "s30.h"
#include "bluetooth.h"
#include <string.h>

LOG_MODULE_REGISTER(s30, LOG_LEVEL_INF);

#define SCD30_NODE DT_NODELABEL(scd30)
static const struct i2c_dt_spec scd30_dev = I2C_DT_SPEC_GET(SCD30_NODE);

extern struct k_work_q sensor_wq;

// CRC-8 calculation (Sensirion polynomial 0x31, init 0xFF)
uint8_t scd30_calculate_crc(const uint8_t *data, size_t len) {
    uint8_t crc = 0xFF;
    for (size_t i = 0; i < len; i++) {
        crc ^= data[i];
        for (int b = 0; b < 8; b++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x31;
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Low-level helper: write a 16-bit command to SCD30
static int scd30_write_command(const struct i2c_dt_spec *dev, uint16_t command) {
    uint8_t buf[2] = {command >> 8, command & 0xFF};
    return i2c_write_dt(dev, buf, sizeof(buf));
}

// Low-level helper: write a 16-bit command with a 16-bit argument and CRC to SCD30
static int scd30_write_command_with_arg(const struct i2c_dt_spec *dev, uint16_t command, uint16_t arg) {
    uint8_t buf[5];
    buf[0] = command >> 8;
    buf[1] = command & 0xFF;
    buf[2] = arg >> 8;
    buf[3] = arg & 0xFF;
    buf[4] = scd30_calculate_crc(&buf[2], 2);
    return i2c_write_dt(dev, buf, sizeof(buf));
}

// Low-level helper: write a command and read response bytes (without repeated start, using delay)
static int scd30_read_data(const struct i2c_dt_spec *dev, uint16_t command, uint8_t *data, size_t len) {
    int ret;
    uint8_t cmd_buf[2] = {command >> 8, command & 0xFF};
    
    ret = i2c_write_dt(dev, cmd_buf, sizeof(cmd_buf));
    if (ret < 0) {
        return ret;
    }
    
    // SCD30 requires a short delay to process and prepare response data
    k_msleep(5);
    
    return i2c_read_dt(dev, data, len);
}

// Check if a new measurement is available
static int scd30_is_data_ready(const struct i2c_dt_spec *dev, bool *ready) {
    uint8_t buf[3];
    int ret = scd30_read_data(dev, SCD30_CMD_DATA_READY, buf, sizeof(buf));
    if (ret < 0) {
        return ret;
    }
    
    // Check CRC-8
    if (scd30_calculate_crc(buf, 2) != buf[2]) {
        LOG_ERR("SCD30 Data Ready CRC error!");
        return -EBADMSG;
    }
    
    uint16_t status = ((uint16_t)buf[0] << 8) | buf[1];
    *ready = (status == 1);
    return 0;
}

// Read the latest CO2, Temperature, and Humidity measurements
static int scd30_read_measurement(const struct i2c_dt_spec *dev, float *co2, float *temp, float *hum) {
    uint8_t buf[18];
    int ret = scd30_read_data(dev, SCD30_CMD_READ_MEAS, buf, sizeof(buf));
    if (ret < 0) {
        return ret;
    }
    
    // Validate CRCs of the 6 data blocks (each block is 2 bytes of data + 1 byte CRC)
    for (int i = 0; i < 18; i += 3) {
        if (scd30_calculate_crc(&buf[i], 2) != buf[i + 2]) {
            LOG_ERR("SCD30 Measurement CRC error at index %d!", i);
            return -EBADMSG;
        }
    }
    
    // Reconstruct CO2 (32-bit float)
    uint32_t co2_bin = ((uint32_t)buf[0] << 24) | ((uint32_t)buf[1] << 16) | ((uint32_t)buf[3] << 8) | buf[4];
    memcpy(co2, &co2_bin, sizeof(float));
    
    // Reconstruct Temperature (32-bit float)
    uint32_t temp_bin = ((uint32_t)buf[6] << 24) | ((uint32_t)buf[7] << 16) | ((uint32_t)buf[9] << 8) | buf[10];
    memcpy(temp, &temp_bin, sizeof(float));
    
    // Reconstruct Humidity (32-bit float)
    uint32_t hum_bin = ((uint32_t)buf[12] << 24) | ((uint32_t)buf[13] << 16) | ((uint32_t)buf[15] << 8) | buf[16];
    memcpy(hum, &hum_bin, sizeof(float));
    
    return 0;
}

// Initialize the SCD30 sensor and start continuous measurement
int s30_init(void) {
    if (!device_is_ready(scd30_dev.bus)) {
        LOG_ERR("SCD30 I2C bus device not ready!");
        return -ENODEV;
    }
    
    // Give the sensor time to start up completely
    k_msleep(100);
    
    // Trigger continuous measurement without pressure compensation (0 mbar argument)
    int ret = scd30_write_command_with_arg(&scd30_dev, SCD30_CMD_TRIGGER_MEAS, 0);
    if (ret < 0) {
        LOG_ERR("Failed to start SCD30 continuous measurement: %d", ret);
        return ret;
    }
    
    LOG_INF("SCD30 CO2 sensor initialized and continuous measurement started");
    return 0;
}

// Periodic workqueue handler
void s30_periodic_work_handler(struct k_work *work) {
    extern struct k_work_delayable s30_periodic_work;
    bool ready = false;
    
    int ret = scd30_is_data_ready(&scd30_dev, &ready);
    if (ret == 0 && ready) {
        float co2 = 0.0f;
        float temp = 0.0f;
        float hum = 0.0f;
        
        ret = scd30_read_measurement(&scd30_dev, &co2, &temp, &hum);
        if (ret == 0) {
            LOG_INF("CO2: %.1f ppm | Temp: %.1f °C | Hum: %.1f %%", co2, temp, hum);
            
            // Trigger BLE alert if CO2 PPM level exceeds 1000
            if (co2 > 1000.0f) {
                ble_update_bit_trigger(1, 1);
            } else {
                ble_update_bit_trigger(1, 0);
            }
        } else {
            LOG_WRN("Failed to read SCD30 measurement (err %d)", ret);
        }
    } else if (ret < 0) {
        LOG_WRN("Failed to check SCD30 data ready status (err %d)", ret);
    } else {
        // Data not ready yet, standard behavior, wait for next period
        LOG_DBG("SCD30 data not ready yet");
    }
    
    k_work_schedule_for_queue(&sensor_wq, &s30_periodic_work, K_MSEC(MEASURE_S30_PERIOD_MS));
}
