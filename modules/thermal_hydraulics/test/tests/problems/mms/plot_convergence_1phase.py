#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script is used to generate a convergence plot for a 1-phase problem.
# Input and output file names are specified below. The input file is assumed to
# have the following named CSV columns (in any order):
#   dx
#   rho_err
#   vel_err
#   p_err

import matplotlib.pyplot as plt
from thm_utilities import readCSVFile

input_file = "convergence.csv"
output_file = "convergence.png"

data = readCSVFile(input_file)

dx = data["dx"]
rho_err = data["rho_err"]
vel_err = data["vel_err"]
p_err = data["p_err"]

n_runs = len(rho_err)
slope_1_ref = [2e-6/2**i for i in range(n_runs)] # adjust initial error as necessary
slope_2_ref = [1e-7/4**i for i in range(n_runs)] # adjust initial error as necessary

plt.figure(figsize=(8, 6))
plt.rc('text', usetex=True)
plt.rc('font', family='sans-serif')
ax = plt.subplot(1, 1, 1)
ax.get_yaxis().get_major_formatter().set_useOffset(False)
plt.xlabel("Mesh size, $h$")
plt.ylabel("Error, $\\|e\\|_1$")
plt.loglog(dx, slope_1_ref, linestyle='-', marker='', color='black', label='$m=1$')
plt.loglog(dx, slope_2_ref, linestyle='-', marker='', color='gray', label='$m=2$')
plt.loglog(dx, rho_err, linestyle='--', marker='x', color='indianred', label='$\\|\\rho - \\rho_h\\|_1$')
plt.loglog(dx, vel_err, linestyle='--', marker='o', color='orange', label='$\\|u - u_h\\|_1$')
plt.loglog(dx, p_err, linestyle='--', marker='s', color='lightgreen', label='$\\|p - p_h\\|_1$')
ax.legend(frameon=False, prop={'size':12}, loc='lower right')
plt.tight_layout()
plt.savefig(output_file, dpi=300)
