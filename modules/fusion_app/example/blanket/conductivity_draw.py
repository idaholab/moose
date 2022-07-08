import csv
import numpy as np
import matplotlib.pyplot as plt

FONTSIZE = 14

# F82H
# Temperature
x = [293.93, 390.03, 491.33, 592.63, 696.53, 779.64, 865.36, 943.28, 1008.21, 1067.96, 1140.68] # K
# Conductivity
y = [24.46, 23.04, 21.33, 19.24, 16.77, 14.49, 11.93, 9.37, 6.80, 4.34, 1.30] # W/mK

print (x)
print (y)

# Breeder
x1 = [273.23, 307.47, 362.71, 461.40, 562.54, 666.43, 767.05, 867.29, 967.50, 1070.53, 1170.73, 1268.15, 1381.42, 1480.19] # K
y1 = [3.96, 3.66, 3.43, 2.86, 2.49, 2.20, 2.00, 1.92, 1.85, 1.85, 1.78, 1.64, 1.26, 0.66] # W/mK

# multiplier
x2 = [266.83, 376.31, 477.36, 662.62, 847.89, 1049.99, 1243.68] #K
y2 = [12.96, 18.31, 19.44, 22.82, 25.07, 32.96, 50.42] # W/mK

#Tungsten
x3 = [365.44, 464.99, 556.82, 660.93, 757.28, 858.82, 947.70, 1115.15, 1254.55, 1343.22] #K
y3 = [178.86, 162.76, 149.22, 142.95, 140.10, 134.69, 129.26, 122.23, 119.01, 117.86] # W/mK

# Plot Data and Ideal
fig, ax = plt.subplots(tight_layout=True)
#ax.plot(depth, power, '-k', marker='o',  markerfacecolor='white', markersize=8, label='MOOSE')
#ax.set_yscale('log')
#ax.plot(depth, power)
ax.plot(x, y, marker='o', markersize=8, label='F82H')
#ax.plot(x1, y1, marker='v')
#ax.plot(x2, y2, marker='*')
#ax.plot(x3, y3, marker='*')

# Beautiful Plot
ax.set_xlabel('Temperature (K)', fontsize=FONTSIZE)
ax.set_ylabel('Thermal Conductivity (W/mK)', fontsize=FONTSIZE)
#ax.set(xticks=[n_procs[0]] + n_procs[2:], ylim=[0.9, 4])
#ax.set(xticklabels=['{:,}'.format(x) for x in ax.get_xticks()])
ax.tick_params(labelsize=FONTSIZE)
ax.grid(True, axis='both')
ax.legend(loc='upper right')

fig.savefig('conductivity_F82H.pdf')

# Plot Data and Ideal
fig, ax = plt.subplots(tight_layout=True)
#ax.plot(depth, power, '-k', marker='o',  markerfacecolor='white', markersize=8, label='MOOSE')
#ax.set_yscale('log')
#ax.plot(depth, power)
#ax.plot(x, y, marker='o')
ax.plot(x1, y1, marker='v', markersize=8, label='Cellular Li2ZrO3')
#ax.plot(x2, y2, marker='*')
#ax.plot(x3, y3, marker='*')

# Beautiful Plot
ax.set_xlabel('Temperature (K)', fontsize=FONTSIZE)
ax.set_ylabel('Thermal Conductivity (W/mK)', fontsize=FONTSIZE)
#ax.set(xticks=[n_procs[0]] + n_procs[2:], ylim=[0.9, 4])
#ax.set(xticklabels=['{:,}'.format(x) for x in ax.get_xticks()])
ax.tick_params(labelsize=FONTSIZE)
ax.grid(True, axis='both')
ax.legend(loc='upper right')

fig.savefig('conductivity_breeder.pdf')

# Plot Data and Ideal
fig, ax = plt.subplots(tight_layout=True)
#ax.plot(depth, power, '-k', marker='o',  markerfacecolor='white', markersize=8, label='MOOSE')
#ax.set_yscale('log')
#ax.plot(depth, power)
#ax.plot(x, y, marker='o')
ax.plot(x2, y2, marker='*', markersize=8, label='Solid Be12Ti')
#ax.plot(x2, y2, marker='*')
#ax.plot(x3, y3, marker='*')

# Beautiful Plot
ax.set_xlabel('Temperature (K)', fontsize=FONTSIZE)
ax.set_ylabel('Thermal Conductivity (W/mK)', fontsize=FONTSIZE)
#ax.set(xticks=[n_procs[0]] + n_procs[2:], ylim=[0.9, 4])
#ax.set(xticklabels=['{:,}'.format(x) for x in ax.get_xticks()])
ax.tick_params(labelsize=FONTSIZE)
ax.grid(True, axis='both')
ax.legend(loc='upper left')

fig.savefig('conductivity_multiplier.pdf')

# Plot Data and Ideal
fig, ax = plt.subplots(tight_layout=True)
#ax.plot(depth, power, '-k', marker='o',  markerfacecolor='white', markersize=8, label='MOOSE')
#ax.set_yscale('log')
#ax.plot(depth, power)
#ax.plot(x, y, marker='o')
ax.plot(x3, y3, marker='^', markersize=8, label='Tungsten')


# Beautiful Plot
ax.set_xlabel('Temperature (K)', fontsize=FONTSIZE)
ax.set_ylabel('Thermal Conductivity (W/mK)', fontsize=FONTSIZE)
#ax.set(xticks=[n_procs[0]] + n_procs[2:], ylim=[0.9, 4])
#ax.set(xticklabels=['{:,}'.format(x) for x in ax.get_xticks()])
ax.tick_params(labelsize=FONTSIZE)
ax.grid(True, axis='both')
ax.legend(loc='upper right')

fig.savefig('conductivity_tungsten.pdf')
