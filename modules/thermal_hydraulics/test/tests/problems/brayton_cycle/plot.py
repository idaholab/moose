# This script generates several plots from either the open- or closed-Brayton
# cycle results.

import matplotlib.pyplot as plt
import numpy as np
from math import pi
from thm_utilities import readCSVFile

open_cycle = True

if open_cycle:
  data_file = 'open_brayton_cycle.csv'
  suffix = '_open.png'
else:
  data_file = 'closed_brayton_cycle.csv'
  suffix = '_closed.png'

data = readCSVFile(data_file)

time = data['time']
shaft_speed = data['shaft_speed']
heating_rate = data['heating_rate']
motor_power = data['motor_power']
generator_power = data['generator_power']

torque_isen_comp = data['compressor:isentropic_torque']
torque_diss_comp = data['compressor:dissipation_torque']
torque_fric_comp = data['compressor:friction_torque']
power_comp = (torque_isen_comp + torque_diss_comp + torque_fric_comp) * shaft_speed
rp_comp = data['p_ratio_comp']
mfr_comp = data['mfr_comp']

torque_isen_turb = data['turbine:isentropic_torque']
torque_diss_turb = data['turbine:dissipation_torque']
torque_fric_turb = data['turbine:friction_torque']
power_turb = (torque_isen_turb + torque_diss_turb + torque_fric_turb) * shaft_speed
rp_turb = data['p_ratio_turb']
mfr_turb = data['mfr_turb']

motor_ramp_up_duration = 100.0
motor_ramp_down_duration = 100.0
t1 = motor_ramp_up_duration
t2 = t1 + motor_ramp_down_duration

# Pressure ratio vs. time
plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Time [s]")
plt.ylabel("Pressure Ratio")
ax.axvline(x=t1, linestyle='--', color='black', label='$t = t_1$')
ax.axvline(x=t2, linestyle=':', color='black', label='$t = t_2$')
plt.plot(time, rp_comp, linestyle='-',  marker='', color='indianred', label='Compressor')
plt.plot(time, rp_turb, linestyle='--', marker='', color='cornflowerblue', label='Turbine')
ax.legend(frameon=False, prop={'size':12}, loc='center left')
plt.tight_layout()
plt.savefig('p_ratio_vs_time' + suffix, dpi=300)

# Shaft speed vs. time
plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Time [s]")
plt.ylabel("Mass Flow Rate [kg/s]")
ax.axvline(x=t1, linestyle='--', color='black', label='$t = t_1$')
ax.axvline(x=t2, linestyle=':', color='black', label='$t = t_2$')
plt.plot(time, mfr_comp, linestyle='-',  marker='', color='indianred', label='Compressor')
plt.plot(time, mfr_turb, linestyle='--', marker='', color='cornflowerblue', label='Turbine')
ax.legend(frameon=False, prop={'size':12}, loc='center left')
plt.tight_layout()
plt.savefig('mfr_vs_time' + suffix, dpi=300)

# Shaft speed vs. time
plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Time [s]")
plt.ylabel("Shaft Speed [rpm]")
ax.axvline(x=t1, linestyle='--', color='black', label='$t = t_1$')
ax.axvline(x=t2, linestyle=':', color='black', label='$t = t_2$')
plt.plot(time, shaft_speed / (2 * pi / 60), linestyle='-', marker='', color='black', label='$\\omega$')
ax.legend(frameon=False, prop={'size':12}, loc='lower right')
plt.tight_layout()
plt.savefig('shaft_speed_vs_time' + suffix, dpi=300)

# Power vs. time
plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Time [s]")
plt.ylabel("Power [kW]")
ax.axvline(x=t1, linestyle='--', color='black', label='$t = t_1$')
ax.axvline(x=t2, linestyle=':', color='black', label='$t = t_2$')
plt.plot(time, heating_rate / 1e3, linestyle='-', marker='', color='indianred', label='$\\dot{Q}_{in}$')
if not open_cycle:
  cooling_rate = data['cooling_rate']
  plt.plot(time, -cooling_rate / 1e3, linestyle='--', marker='', color='cornflowerblue', label='$-\\dot{Q}_{out}$')
plt.plot(time, -generator_power / 1e3, linestyle='-', marker='', color='lightgreen', label='$-\\dot{W}_{gen}$')
plt.plot(time, -power_comp / 1e3, linestyle='-', marker='', color='gold', label='$-\\dot{W}_{comp}$')
plt.plot(time, power_turb / 1e3, linestyle='--', marker='', color='mediumpurple', label='$\\dot{W}_{turb}$')
plt.plot(time, (power_turb + power_comp) / 1e3, linestyle='--', marker='', color='magenta', label='$\\dot{W}_{turb} + \\dot{W}_{comp}$')
ax.legend(frameon=False, prop={'size':12}, loc='upper left')
plt.tight_layout()
plt.savefig('power_vs_time' + suffix, dpi=300)
