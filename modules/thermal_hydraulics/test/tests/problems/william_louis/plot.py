#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import matplotlib.pyplot as plt
import pandas as pd

p_low = 100

def plotCase(test, pipe_loc_strs):
  thm_file_name = test + '.csv'
  data = pd.read_csv(thm_file_name)

  for pipe_loc_str in pipe_loc_strs:
    exp_file_name = 'data/' + test + '_' + pipe_loc_str + '.csv'
    exp_data = pd.read_csv(exp_file_name)
    exp_data = exp_data.sort_values(by=['time'])

    pp_name = 'p_' + pipe_loc_str
    plot_name = test + '_' + pipe_loc_str + '.png'

    plt.figure(figsize=(8, 6))
    plt.rc('text', usetex=True)
    plt.rc('font', family='sans-serif')
    ax = plt.subplot(1, 1, 1)
    ax.get_yaxis().get_major_formatter().set_useOffset(False)
    plt.xlabel("Time, $t$ [s]")
    plt.ylabel("Pressure Difference, $\\Delta p$ [kPa]")
    plt.plot(exp_data['time'], exp_data['dp'], linestyle='-', marker='', color='black', label='Experiment')
    plt.plot(data['time'], data[pp_name] / 1e3 - p_low, linestyle='--', marker='', color='cornflowerblue', label='THM')
    ax.legend(frameon=False, prop={'size':12})
    plt.tight_layout()
    plt.savefig(plot_name, dpi=300)

plotCase('4pipes_closed', ['pipe1_048'])
plotCase('3pipes_open', ['pipe1_048', 'pipe2_052', 'pipe3_048'])
