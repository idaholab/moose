# This script generates several plots from the recuperated brayton cycle solution

import matplotlib.pyplot as plt
import numpy as np
from math import pi
from thm_utilities import readCSVFile

data_file = 'recuperated_brayton_cycle.csv'
suffix = '_recuperated.png'

data = readCSVFile(data_file)

end_time = 2000
for i in range(len(data['time'])):
    if data['time'][i] >= end_time:
        end_time_index = i
        break

time = data['time']


torque_isen_comp = data['compressor:isentropic_torque']
torque_diss_comp = data['compressor:dissipation_torque']
torque_fric_comp = data['compressor:friction_torque']
rp_comp = data['p_ratio_comp']

torque_isen_turb = data['turbine:isentropic_torque']
torque_diss_turb = data['turbine:dissipation_torque']
torque_fric_turb = data['turbine:friction_torque']
rp_turb = data['p_ratio_turb']

cold_leg_in = data['cold_leg_in']
cold_leg_out = data['cold_leg_out']
hot_leg_in = data['hot_leg_in']
hot_leg_out = data['hot_leg_out']

T_in_comp = data['T_in_comp']
T_out_comp = data['T_out_comp']
T_in_turb = data['T_in_turb']
T_out_turb = data['T_out_turb']

shaft_RPM = data['shaft_RPM']
motor_torque = data['motor_torque']
turbine_torque = data['turbine_torque']

# Torque comparison
plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Time [s]")
plt.ylabel("Torque [N-m]")
plt.plot(time, motor_torque, linestyle='-',  marker='', color='indianred', label='Motor')
plt.plot(time, turbine_torque, linestyle='--', marker='', color='cornflowerblue', label='Turbine')
ax.legend(frameon=False, prop={'size':12}, loc='center right')
plt.tight_layout()
plt.savefig('torque_comparison' + suffix, dpi=300)

# Pressure ratio comparison
plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Time [s]")
plt.ylabel("Pressure Ratio")
plt.plot(time, rp_comp, linestyle='-',  marker='', color='indianred', label='Compressor')
plt.plot(time, rp_turb, linestyle='--', marker='', color='cornflowerblue', label='Turbine')
ax.legend(frameon=False, prop={'size':12}, loc='center right')
plt.tight_layout()
plt.savefig('pressure_ratio_comparison' + suffix, dpi=300)

# PID startup
plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Time [s]")
plt.ylabel("Shaft Speed [rpm]")
plt.plot(time, shaft_RPM, linestyle='-', marker='', color='black', label='$\\omega$')
plt.xlim([0, 2000])
plt.ylim([0, 85000])
plt.tight_layout()
plt.savefig('PID_startup' + suffix, dpi=300)

# Shaft speed vs. time
plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Time [s]")
plt.ylabel("Shaft Speed [rpm]")
plt.plot(time, shaft_RPM, linestyle='-', marker='', color='black', label='$\\omega$')
plt.tight_layout()
plt.savefig('shaft_speed_vs_time' + suffix, dpi=300)

# Temperature comparison
plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Time [s]")
plt.ylabel("Temperature [K]")
plt.plot(time, T_in_turb, linestyle='-', marker='', color='indianred', label='Turbine Inlet')
plt.plot(time, T_out_turb, linestyle='-', marker='', color='forestgreen', label='Turbine Outlet')
plt.plot(time, cold_leg_out, linestyle='--', marker='', color='mediumpurple', label='Cold Leg Outlet')
plt.plot(time, hot_leg_out, linestyle='--', marker='', color ='darkslategrey', label='Hot Leg Outlet')
plt.plot(time, T_out_comp, linestyle='-', marker='', color='blue', label='Compressor Outlet')
ax.legend(frameon=False, prop={'size':12}, loc='best')
plt.tight_layout()
plt.savefig('temperature_comparison' + suffix, dpi=300)
