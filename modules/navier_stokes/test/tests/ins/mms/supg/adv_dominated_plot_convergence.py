# To compare convergence of INS variables for SUPG stabilized and unstabilized cases
# run the run_adv_dominated.py script and then run this script

import numpy as np
import matplotlib.pyplot as plt

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

h_array = np.array([.25, .125, .0625, .03125, .015625])
variable_names = ['L2p', 'L2vel_x', 'L2vel_y', 'L2vxx']
strings = ["stabilized", "unstabilized"]
mu = "1.5e-2"

for name in variable_names:
    for string in strings:
        data_array = np.array([])
        for h in h_array:
            n = int(1 / h)
            data_file = "%s_mu%s_%sx%s.csv" % (string, mu, n, n)
            data = np.genfromtxt(data_file, dtype=float, names=True, delimiter=',')
            data_array = np.append(data_array, data[name])
        z = np.polyfit(np.log10(h_array), np.log10(data_array), 1)
        p = np.poly1d(z)
        plt.plot(np.log10(h_array), p(np.log10(h_array)), '-')
        equation = "y=%.1fx+%.1f" % (z[0], z[1])
        plt.scatter(np.log10(h_array), np.log10(data_array), label=r"$%s; %s$" % (string, equation))
    plt.xlabel(r"$\log_{10} h$")
    plt.ylabel(r"$\log_{10} L_2$")
    plt.legend(prop=fontP)
    plt.tight_layout()
    plt.savefig("adv_dominated_convergence_plot_%s.eps" % name, format='eps')
    plt.close()
