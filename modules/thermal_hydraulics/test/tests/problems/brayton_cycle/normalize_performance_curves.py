# This script normalizes the raw compressor and turbine performance curves
# and saves them to CSV files. Also, a plot of the performance curves is generated.

import matplotlib.pyplot as plt
import numpy as np
from thm_utilities import readCSVFile, writeCSVFile

# Tuples of (shaft speed in [rpm], filename suffix)
cases = [
  (5e4, '50k'),
  (6e4, '60k'),
  (7e4, '70k'),
  (8e4, '80k'),
  (9e4, '90k')
]

# Rated shaft speed in [rpm] and mass flow rate in [kg/s]
speed_rated_rpm = 96000
mfr_rated = 0.25

# Create a list of tuples for each case with the following elements:
#   1. normalized shaft speed
#   2. normalized compressor mass flow rate
#   3. compressor pressure ratio
#   4. normalized turbine mass flow rate
#   5. turbine pressure ratio
data = list()
for i, case in enumerate(cases):
  speed_rpm, speed_str = case
  speed_normalized = speed_rpm / speed_rated_rpm

  rp_comp_data = readCSVFile('rp_comp_' + speed_str + '.csv')
  mfr_comp = rp_comp_data['mass_flow_rate']
  mfr_comp_normalized = mfr_comp / mfr_rated
  rp_comp = rp_comp_data['pressure_ratio']

  rp_turb_data = readCSVFile('rp_turb_' + speed_str + '.csv')
  mfr_turb = rp_turb_data['mass_flow_rate']
  mfr_turb_normalized = mfr_turb / mfr_rated
  rp_turb = rp_turb_data['pressure_ratio']

  data.append((speed_normalized, mfr_comp_normalized, rp_comp, mfr_turb_normalized, rp_turb))

# Create a plot of the turbomachinery curves and write CSV files for the normalized curves
plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Mass Flow Rate, $\\nu$")
plt.ylabel("Pressure Ratio, $r_p$")
ax.set_xlim([0.0, 1.2])
ax.set_ylim([0.5, 4.0])
colors = ['indianred', 'gold', 'lightgreen', 'cornflowerblue', 'mediumpurple']
for i, data_i in enumerate(data):
  speed_normalized, mfr_comp_normalized, rp_comp, mfr_turb_normalized, rp_turb = data_i

  label_comp = 'Compressor, $\\alpha = %.4f$' % speed_normalized
  plt.plot(mfr_comp_normalized, rp_comp, linestyle='-',  marker='', color=colors[i], label=label_comp)

  label_turb = 'Turbine, $\\alpha = %.4f$' % speed_normalized
  plt.plot(mfr_turb_normalized, rp_turb, linestyle='--', marker='', color=colors[i], label=label_turb)

  data_comp = {'mass_flow_rate' : mfr_comp_normalized, 'pressure_ratio' : rp_comp}
  writeCSVFile(data_comp, 'rp_comp%d.csv' % (i+1))

  data_turb = {'mass_flow_rate' : mfr_turb_normalized, 'pressure_ratio' : rp_turb}
  writeCSVFile(data_turb, 'rp_turb%d.csv' % (i+1))

ax.legend(frameon=False, prop={'size':12}, loc='upper left')
plt.tight_layout()
plt.savefig('rp_vs_mfr.png', dpi=300)
