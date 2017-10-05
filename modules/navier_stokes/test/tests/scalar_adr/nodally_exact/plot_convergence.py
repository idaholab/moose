# To compare convergence of INS variables for SUPG stabilized and unstabilized cases
# run the run_adv_dominated.py script and then run this script

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

h_array = np.array([.25,
                    .125,
                    .0625,
                    .03125])
                    # .015625,
                    # .0078125,
                    # .00390625])
variable_names = ['L2u', 'L2ux']
# strings = ['_'.join(filter(None, ('stabilized', sys.argv[1])))]  # , '_'.join(filter(None, ('unstabilized', sys.argv[1])))]
strings = map(lambda x: '_'.join(('stabilized', x)),
              ['edge2', 'edge3', 'quad4', 'quad9', 'transient_quad9'])
error_dict = {'L2u': r'e_{u}', 'L2ux': r'e_{u_x}'}

for name in variable_names:
    for string in strings:
        data_array = np.array([])
        for h in h_array:
            n = int(1 / h)
            data_file = "%s_%sx%s.csv" % (string, n, n)
            data = np.genfromtxt(data_file, dtype=float, names=True, delimiter=',')
            data_array = np.append(data_array, data[name][-1])
        z = np.polyfit(np.log10(h_array), np.log10(data_array), 1)
        p = np.poly1d(z)
        plt.plot(np.log10(h_array), p(np.log10(h_array)), '-')
        equation = "y=%.1fx+%.1f" % (z[0], z[1])
        m = "%.1f" % z[0]
        plt.scatter(np.log10(h_array), np.log10(data_array),
                    label=r"$%s$; %s" % (m, ' '.join(string.split('_')[1:])))
    plt.xlabel(r"$\log_{10} h$")
    plt.ylabel(r"$\log_{10} ||%s||$" % error_dict[name])
    plt.legend(prop=fontP)
    plt.tight_layout()
    plt.savefig('_'.join(filter(None, string.split('_')[1:] + [name])) + "_element_comparison.eps", format='eps')
    plt.close()
