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
    def __init__(self, var_names, method, h_array, error_dict):
        self.var_names = var_names
        self.method = method
        self.h_array = h_array
        self.error_dict = error_dict

    def draw_single_line(self, data_array, var_name):
        z = np.polyfit(np.log10(self.h_array), np.log10(data_array), 1)
        p = np.poly1d(z)
        plt.plot(np.log10(self.h_array), p(np.log10(self.h_array)), '-')
        equation = "y=%.1fx+%.1f" % (z[0], z[1])
        m = "%.1f" % z[0]
        plt.scatter(np.log10(self.h_array), np.log10(data_array),
                label=r"$%s$; $%s$" % (m, self.error_dict[var_name]))

    def label_and_save(self):
        plt.xlabel(r"$\log_{10} h$")
        plt.ylabel(r"$\log_{10} ||e||$")
        plt.legend(prop=fontP)
        plt.tight_layout()
        plt.savefig('_'.join(filter(None, method.split('_')[1:])) + "_combined_names_scalar.eps", format='eps')
        plt.close()


    def loop_h(self, var_name, operation):
        data_array = np.array([])
        for h in self.h_array:
            n = int(1 / h)
            data_file = "%s_%sx%s.csv" % (self.method, n, n)
            data = np.genfromtxt(data_file, dtype=float, names=True, delimiter=',')
            data_array = np.append(data_array, data[var_name][-1])
            if operation == "write":
                self.write_object.write(str(data[var_name][-1]) + '\n')
        if operation == "plot":
            self.draw_single_line(data_array, var_name)


    def loop_vars(self, operation="plot"):
        if operation not in {"plot", "write"}:
            raise ValueError("Valid options for operation are 'plot' and 'write'")
        for name in self.var_names:
            self.loop_h(name, operation)
        if operation == "plot":
            self.label_and_save()


    def write_data(self, file_name):
        self.write_object = open(file_name, 'w')
        self.loop_vars(operation="write")
        self.write_object.close()

    def plot_data(self):
        self.loop_vars(operation="plot")


h_array = np.array([.5,
                    .25,
                    .125,
                    .0625,
                    .03125])
                    # .015625])
                    # .0078125,
                    # .00390625])
variable_names = ['L2u', 'L2ux']
methods = sys.argv[1:] if 1 < len(sys.argv) else []
methods = map(lambda x: '_'.join(filter(None, ('stabilized', x))), methods)
error_dict = {'L2u': r'\|u-u_h\|_{L^2}', 'L2ux': r'\|u-u_h\|_{H^1}'}
operation = "write"
for method in methods:
    plot = GeneratePlot(variable_names, method, h_array, error_dict)
    if operation == "plot":
        plot.plot_data()
    elif operation == "write":
        plot.write_data(method + ".txt")
