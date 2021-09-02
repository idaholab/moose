import numpy as np
import matplotlib.pyplot as plt
import matplotlib as mpl
import csv
import math
import scipy

fig1, ax1 = plt.subplots()

y = []
contact_pressure = []
with open('cylinder_friction_nodetosegment_penalty_out_cont_press_0001.csv') as csv_file: #293
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            # print(f'Column names are {", ".join(row)}')
            line_count += 1
        else:
            contact_pressure.append(float(row[0]))
            y.append(float(row[2]))
            # print(f'\t{row[0]} works in the {row[1]} department, and was born in {row[2]}.')
            line_count += 1
    print(f'Processed {line_count} lines.')

contact_pressure_ = [x for _,x in sorted(zip(y,contact_pressure))]
contact_pressure = contact_pressure_
y.sort()
ax1.plot(y,contact_pressure, 'b', linewidth=2, label='Contact pressure (NTS)')


nodal_area = []
with open('cylinder_friction_nodetosegment_penalty_out_nodal_area_post_0001.csv') as csv_file: #293
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            # print(f'Column names are {", ".join(row)}')
            line_count += 1
        else:
            nodal_area.append(float(row[1]))
            # print(f'\t{row[0]} works in the {row[1]} department, and was born in {row[2]}.')
            line_count += 1
    print(f'Processed {line_count} lines.')



y = []
frictional_pressure = []
with open('cylinder_friction_nodetosegment_penalty_out_friction_force_0001.csv') as csv_file: #293
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            # print(f'Column names are {", ".join(row)}')
            line_count += 1
        else:
            frictional_pressure.append(float(row[1])/nodal_area[line_count-1])
            y.append(float(row[2]))
            # print(f'\t{row[0]} works in the {row[1]} department, and was born in {row[2]}.')
            line_count += 1
    print(f'Processed {line_count} lines.')

frictional_pressure_ = [x for _,x in sorted(zip(y,frictional_pressure))]
frictional_pressure = frictional_pressure_
y.sort()
ax1.plot(y,frictional_pressure, 'r', linewidth=2, label='Frictional pressure (NTS)')

ax1.set_xlabel('Interface longitudinal dimension [$m$]')
ax1.set_ylabel('Contact pressure [$MPa$]')
ax1.grid()

## Mortar ##
y = []
contact_pressure = []
with open('cylinder_friction_mortar_out_cont_press_0001.csv') as csv_file: #293
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            # print(f'Column names are {", ".join(row)}')
            line_count += 1
        else:
            contact_pressure.append(float(row[0]))
            y.append(float(row[2]))
            # print(f'\t{row[0]} works in the {row[1]} department, and was born in {row[2]}.')
            line_count += 1
    print(f'Processed {line_count} lines.')

contact_pressure_ = [x for _,x in sorted(zip(y,contact_pressure))]
contact_pressure = contact_pressure_
y.sort()
ax1.plot(y,contact_pressure, 'g', linewidth=2, label='Contact pressure (mortar)')

y = []
frictional_pressure = []
with open('cylinder_friction_mortar_out_friction_0001.csv') as csv_file: #293
    csv_reader = csv.reader(csv_file, delimiter=',')
    line_count = 0
    for row in csv_reader:
        if line_count == 0:
            # print(f'Column names are {", ".join(row)}')
            line_count += 1
        else:
            frictional_pressure.append(float(row[1]))
            y.append(float(row[2]))
            # print(f'\t{row[0]} works in the {row[1]} department, and was born in {row[2]}.')
            line_count += 1
    print(f'Processed {line_count} lines.')


frictional_pressure_ = [x for _,x in sorted(zip(y,frictional_pressure))]
frictional_pressure = frictional_pressure_
y.sort()
ax1.plot(y,frictional_pressure, 'k', linewidth=2, label='Frictional pressure (mortar)')


# # Analytical results
# N = 10342.693202616
# T = 2.2853328118799e-09
# Radius = 2.0
# pi = 3.1415926535
# mu = 0.4
# E_star = 1.0e6/(1-0.3*0.3)
# a = math.sqrt(4.0 * N * Radius/pi/E_star)
# print(a)
# print(N)
# print(T)
# print(2 * N/pi/a)
# c = (1 - T/mu/N)*a
# # 100 linearly spaced numbers
# x = np.arange(-a,a,1.0e-4)
# press_analytical = 2 * N/pi/a * np.sqrt(1 - x*x/a/a)
# print(press_analytical)
# ax1.plot(x,press_analytical, 'y--', linewidth=2, label='Contact pressure (analytical)')
#
# # friction_lm
# def friction (c, a, x):
#     sat = 2 * N * mu/pi/a *  np.sqrt(1 - x*x/a/a)
#     if c < np.abs(x) <= a:
#         return sat
#     elif np.abs(x) <= c:
#         return sat - 2.0/pi/c * mu * N * (c*c/a/a)*np.sqrt(1 - x*x/c/c)
#     return 0.0
#
# vfun = np.vectorize(friction)
# fric_analytical = vfun(c, a, x)
# ax1.plot(x,fric_analytical, 'b--', linewidth=2, label='Frictional pressure (analytical)')



# Analytical results




ax1.set_xlabel('Interface longitudinal dimension [$m$]')
ax1.set_ylabel('Contact pressure [$MPa$]')
ax1.grid()

ax1.set_xlim(-0.2, 0.2)
# ax1.set_ylim(-40, 100)

plt.legend(loc='best')
plt.savefig('contact.pdf')
plt.show()
