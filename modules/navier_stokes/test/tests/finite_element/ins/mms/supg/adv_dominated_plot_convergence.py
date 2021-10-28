#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# Example script for plotting MMS cases

import sys
import numpy as np
import matplotlib.pyplot as plt
from itertools import chain

# Use fonts that match LaTeX
from matplotlib import rcParams
from matplotlib.font_manager import FontProperties

rcParams['font.family'] = 'serif'
rcParams['font.size'] = 17
rcParams['font.serif'] = ['Computer Modern Roman']
rcParams['text.usetex'] = True

# Small font size for the legend
fontP = FontProperties()
fontP.set_size('x-small')

class GeneratePlot():
    def __init__(self, mus, var_names, method, h_array, error_dict):
        self.mus = mus
        self.var_names = var_names
        self.method = method
        self.h_array = h_array
        self.error_dict = error_dict

    def draw_single_line(self, data_array, mu, var_name):
        z = np.polyfit(np.log10(self.h_array), np.log10(data_array), 1)
        p = np.poly1d(z)
        plt.plot(np.log10(self.h_array), p(np.log10(self.h_array)), '-')
        equation = "y=%.1fx+%.1f" % (z[0], z[1])
        m = "%.1f" % z[0]
        mu_formatted = "%2.1E" % float(mu)
        mu_dec = mu_formatted[:3]
        mu_exp = ''.join(filter(lambda x: x != '+' and x != '0', list(mu_formatted[4:])))
        plt.scatter(np.log10(self.h_array), np.log10(data_array),
                label=r"$%s$; $\|%s\|$" % (m, self.error_dict[var_name]))

    def label_and_save(self, mu):
        plt.xlabel(r"$\log_{10} h$")
        plt.ylabel(r"$\log_{10} ||e||$")
        plt.legend(prop=fontP)
        plt.tight_layout()
        plt.savefig('_'.join(filter(None, method.split('_')[1:] + ["mu" + mu])) + "_combined_names.eps", format='eps')
        plt.close()


    def loop_h(self, mu, var_name, operation):
        data_array = np.array([])
        for h in self.h_array:
            n = int(1 / h)
            data_file = "%s_mu%s_%sx%s.csv" % (self.method, mu, n, n)
            data = np.genfromtxt(data_file, dtype=float, names=True, delimiter=',')
            data_array = np.append(data_array, data[var_name])
            if operation == "write":
                self.write_object.write(str(data[var_name]) + '\n')
        if operation == "plot":
            self.draw_single_line(data_array, mu, var_name)


    def mus_then_vars(self, operation="plot"):
        if operation not in {"plot", "write"}:
            raise ValueError("Valid options for operation are 'plot' and 'write'")
        for mu in self.mus:
            for name in self.var_names:
                self.loop_h(mu, name, operation)
            if operation == "plot":
                self.label_and_save(mu)


    def write_data(self, file_name):
        self.write_object = open(file_name, 'w'):
        self.mus_then_vars(operation="write")
        self.write_object.close()

    def plot_data(self):
        self.mus_then_vars(operation="plot")


h_array = np.array([.25,
                    .125,
                    .0625,
                    .03125])
                    # .015625])
                    # .0078125,
                    # .00390625])
variable_names = ['L2p', 'L2vel_x', 'L2vel_y', 'L2vxx']
methods = sys.argv[1:] if 1 < len(sys.argv) else []
methods = map(lambda x: '_'.join(filter(None, ('stabilized', x))), methods)
mus = ["1.5e-4", "1.5e1"]
error_dict = {'L2p': r'e_p', 'L2vel_x': r'e_{v_x}', 'L2vel_y': r'e_{v_y}', 'L2vxx': r'\nabla e_{v_x}'}
for method in methods:
    plot = GeneratePlot(mus, variable_names, method, h_array, error_dict)
    plot.plot_data()
