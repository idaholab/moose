# This script plots the pressure drop solutions from various spatial discretization
# options against an analytic solution. Before executing this script, execute
# the 'run_tests' script with the input file 'spatial_discretization_study':
#   run_tests -i spatial_discretization_study

from math import pi, sqrt
import matplotlib.pyplot as plt
import numpy as np

from thm_utilities import readCSVFile

f = 1e4
mfr = 1.0
A = 0.5 * 0.353504
L = 1.0
TL = 300.0
pR = 7e6

Dh = sqrt(4 * A / pi)

Ru = 8.3144598
gamma = 1.3066
M = 2.016e-3
R = Ru / M
cp = gamma * R / (gamma - 1.0)
cv = cp / gamma

def computePressureDropIsothermalNoDynamic(x):
  pL = sqrt(pR**2 + f * mfr**2 * R * TL * x / (Dh * A**2))
  return pL - pR

x_analytic = np.linspace(0, L, 100)
dp_analytic = [computePressureDropIsothermalNoDynamic(x) for x in x_analytic]

schemes = [
  ('none', 'None', 'darkred', 'lightcoral'),
  ('full', 'Full', 'royalblue', 'lightsteelblue'),
  ('minmod', 'Minmod', 'forestgreen', 'greenyellow'),
  ('mc', 'MC', 'darkorchid', 'mediumpurple'),
  ('superbee', 'Superbee', 'darkorange', 'gold')]
sets = []
for scheme, scheme_label, color1, color2 in schemes:
  sets.append((scheme + '_pressure_vpp_FINAL.csv', '-', color1, scheme_label + ', No junction'))
  sets.append((scheme + '_junc_pressure_vpp_FINAL.csv', '--', color2, scheme_label + ', With junction'))

plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Position [m]")
plt.ylabel("Pressure [Pa]")
plt.plot(x_analytic, dp_analytic, '-', color='black', marker='', label='Analytic')
for set in sets:
  filename, linestyle, color, label = set
  data = readCSVFile(filename)
  x = data['x']
  p = data['p']
  p_ref = p[0] - x[0] * (p[1] - p[0]) / (x[1] - x[0])
  dp = p_ref - p
  plt.plot(x, dp, linestyle, color=color, marker='', label=label)
ax.legend(frameon=True, prop={'size':10})
plt.tight_layout()
plt.savefig('pressure_drop.png', dpi=300)
