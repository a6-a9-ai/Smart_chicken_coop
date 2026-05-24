#include "uart.h"

LOG_MODULE_REGISTER(uart_custom, LOG_LEVEL_INF);

/* Using uart20 on nRF54L15DK:
 * RX: P1.05
 * TX: P1.04 */

#define UART_DEVICE_NODE DT_NODELABEL(uart20)
static const struct device *uart_dev = DEVICE_DT_GET(UART_DEVICE_NODE);

static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data){
    switch (evt->type) {
    case UART_RX_RDY:
        if (evt->data.rx.len > 0) {
            LOG_INF("UART Received %d bytes", evt->data.rx.len);
            for (int i = 0; i < evt->data.rx.len; i++) {
                uint8_t byte = evt->data.rx.buf[evt->data.rx.offset + i];
                LOG_INF("Byte: %c (0x%02X)", byte, byte);
            }
        }
        break;
    case UART_RX_DISABLED:
        uart_rx_enable(dev, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
        break;

    default:
        break;
    }
}

int uart_init(void){
    int ret;
    if (!device_is_ready(uart_dev)) {
        LOG_ERR("UART device not ready");
        return -ENODEV;
    }
    ret = uart_callback_set(uart_dev, uart_cb, NULL);
    if (ret) {
        LOG_ERR("Error setting UART callback: %d", ret);
        return ret;
    }
    ret = uart_rx_enable(uart_dev, rx_buf, sizeof(rx_buf), RECEIVE_TIMEOUT);
    if (ret) {
        LOG_ERR("Error enabling UART RX: %d", ret);
        return ret;
    }
    LOG_INF("UART initialized on %s", uart_dev->name);
    return 0;
}
