# -*- coding: utf-8 -*-
"""
Created on Wed Nov 12 15:42:55 2025

@author: E7290
"""

import numpy as np
import matplotlib.pyplot as plt

def calcul_VTS(RT1, RT2, T_celcius):
    vcc = 6
    T0 = 298.15
    return vcc / RT1 / (1/RT1 + 1/RT2 + 1/(10000 * np.exp(4300 * (1/(273.15 + T_celcius) - 1/T0))))

def trace_VTS(RT1=10000, RT2=10000):
    x_T_celcius = [i - 20 for i in range(130)]
    y_VTS = []
    for T_celcius in x_T_celcius:
        y_VTS.append(calcul_VTS(RT1, RT2, T_celcius))
    return x_T_celcius, y_VTS


RT2 = 2000
RT1_values = range(1800, 2001, 25)

plt.figure(figsize=(8, 5))

for RT1 in RT1_values:
    x, y = trace_VTS(RT1, RT2)
    plt.plot(x, y, linewidth=2, label=f"RT1 = {RT1/1000:.0f} kΩ")

plt.axhline(y=3, color='gray', linestyle='--', linewidth=1.5)
plt.axhline(y=1.5, color='gray', linestyle='--', linewidth=1.5)

plt.axvline(x=0, color='gray', linestyle='--', linewidth=1.5)
plt.axvline(x=80, color='gray', linestyle='--', linewidth=1.5)

plt.title(f"Évolution de la tension VTS selon RT1 (RT2 = {RT2/1000:.1f} kΩ)", fontsize=14, fontweight='bold')
plt.xlabel("Température mesurée (°C)", fontsize=12)
plt.ylabel("Tension VTS (V)", fontsize=12)

plt.grid(True, linestyle="--", alpha=0.6)
plt.legend(title="Valeurs de RT1 / Limites", fontsize=10, title_fontsize=11, loc="best")
plt.tight_layout()
plt.show()
