import matplotlib.pyplot as plt
import numpy as np
import pandas as pd

dataset1 = pd.read_csv('simple_blanket_heat_transfer_out_temp_y_0001.csv')
df1 = pd.DataFrame(dataset1)
cols = [1,3]
df1 = df1[df1.columns[cols]]

d = df1["y"]*100
temp = df1["temp"]

x_FWA = 0.2
x_FW = x_FWA + 3.8
x_M = x_FW + 2.3
x_T1 = x_M + 2.2
x_B = x_T1 + 1.8
x_T2 = x_B + 2.2

p_FWA = 5.5088
p_FW = 0.92456
p_M = 1.28748
p_T1 = 1.26844
p_B = 3.252
p_T2 = 0.84966

x = [0, x_FWA, x_FWA, x_FW, x_FW, x_M, x_M, x_T1, x_T1, x_B, x_B, x_T2]
y = [p_FWA, p_FWA, p_FW, p_FW, p_M, p_M, p_T1, p_T1, p_B, p_B, p_T2, p_T2]

fs = 14
lw = 2

fig, ax1 = plt.subplots()

color = 'black'
ax1.set_xlabel('Distance from first wall (cm)', fontsize = fs)
ax1.set_ylabel('Nuclear Heating (MW/m^3)', color = color, fontsize = fs)
ax1.plot(x, y, color = color, linewidth = lw)
ax1.tick_params(axis = 'y', labelcolor = color, labelsize = fs)
ax1.tick_params(axis = 'x', labelcolor = color, labelsize = fs)
ax1.grid()

ax2 = ax1.twinx()

color = 'red'
ax2.set_ylabel('Temperature (K)', color = color, fontsize = fs)
ax2.plot(d, temp, color = color, linewidth = lw)
ax2.tick_params(axis = 'y', labelcolor = color, labelsize = fs)
fig.tight_layout()
plt.savefig('Power_and_temp' + '.png', format = 'png', dpi = 300)



'''
plt.figure(figsize = (10,8), dpi =300)
plt.plot(x, y, color = 'black', linewidth = lw)
plt.grid()
plt.xticks(fontsize = fs)
plt.yticks(fontsize = fs)
plt.rc('font', size=fs)
plt.xlabel('Distance from first wall (cm)', fontsize = fs)
plt.ylabel('Nuclear Heating (MW/m^3)', fontsize = fs)
plt.tight_layout()
plt.savefig('Powerlevel' + '.png', format = 'png', dpi = 300)
'''