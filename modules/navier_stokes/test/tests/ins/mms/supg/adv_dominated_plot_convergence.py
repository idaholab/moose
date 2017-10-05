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

h_array = np.array([.25,
                    .125,
                    .0625,
                    .03125])
                    # .015625])
                    # .0078125,
                    # .00390625])
variable_names = ['L2p', 'L2vel_x', 'L2vel_y', 'L2vxx']
strings = ['_'.join(filter(None, ('stabilized', sys.argv[1])))]  # , '_'.join(filter(None, ('unstabilized', sys.argv[1])))]
# strings = map(lambda x: '_'.join(('stabilized', x)),
#               ['consistent_pspg_weighting_mu1.5e-4', 'q2q1_consistent_pspg_weighting_mu1.5e-4',
#                'q2q1_no_pspg_mu1.5e-4', 'q2q1_no_pspg_mu1.5e1'])
# mus = ["1.5e1", "1.5e0", "5e-1", "1.5e-1", "5e-2", "1.5e-2", "5e-3", "1.5e-3", "1.5e-4"]
mus = ["1.5e1"]
error_dict = {'L2p': r'e_p', 'L2vel_x': r'e_{v_x}', 'L2vel_y': r'e_{v_y}', 'L2vxx': r'\nabla e_{v_x}'}
mu_looping = True


for name in variable_names:
    for string in strings:
        for mu in mus:
            data_array = np.array([])
            for h in h_array:
                n = int(1 / h)
                if mu_looping:
                    data_file = "%s_mu%s_%sx%s.csv" % (string, mu, n, n)
                else:
                    data_file = "%s_%sx%s.csv" % (string, n, n)
                data = np.genfromtxt(data_file, dtype=float, names=True, delimiter=',')
                data_array = np.append(data_array, data[name])
            z = np.polyfit(np.log10(h_array), np.log10(data_array), 1)
            p = np.poly1d(z)
            plt.plot(np.log10(h_array), p(np.log10(h_array)), '-')
            equation = "y=%.1fx+%.1f" % (z[0], z[1])
            m = "%.1f" % z[0]
            string_list = string.split('_')
            if string_list[1] != "q2q1":
                string = '_'.join([string_list[0]] + ["q1q1"] + string_list[1:])
            if mu_looping:
                mu_formatted = "%2.1E" % float(mu)
                mu_dec = mu_formatted[:3]
                mu_exp = ''.join(filter(lambda x: x != '+' and x != '0', list(mu_formatted[4:])))
                # plt.scatter(np.log10(h_array), np.log10(data_array),
                #         label=r"$\mu=%s\cdot 10^{%s}$; $%s$; %s" % (mu_dec, mu_exp, m, ' '.join(string.split('_')[1:])))
                plt.scatter(np.log10(h_array), np.log10(data_array),
                        label=r"$%s$; $\|%s\|$" % (m, error_dict[name]))
            else:
                plt.scatter(np.log10(h_array), np.log10(data_array),
                        label=r"$%s$; %s; ||%s||" % (m, ' '.join(string.split('_')[1:]), error_dict[name]))

            # plt.scatter(np.log10(h_array), np.log10(data_array),
            #             label=r"$\mu=%s\cdot 10^{%s}$; $%s$; %s" % (mu_dec, mu_exp, m, ' '.join(string.split('_')[1:])))
            # plt.scatter(np.log10(h_array), np.log10(data_array), label=r"$\mu=%s\cdot 10^{%s}$; $%s$" % (mu[:3], mu[4:], equation))
            # plt.scatter(np.log10(h_array), np.log10(data_array), label=r"%s; %s; $%s$" % (string.split('_')[0], mu, equation))
plt.xlabel(r"$\log_{10} h$")
# plt.ylabel(r"$\log_{10} ||%s||$" % error_dict[name])
plt.ylabel(r"$\log_{10} ||e||$")
plt.legend(prop=fontP)
plt.tight_layout()
if mu_looping:
    # plt.savefig('_'.join(filter(None, string.split('_')[1:] + [name, "mu" + mu])) + ".eps", format='eps')
    plt.savefig('_'.join(filter(None, string.split('_')[1:] + ["mu" + mu])) + "_combined_names.eps", format='eps')
else:
    # plt.savefig('_'.join(filter(None, string.split('_')[1:] + [name])) + "_method_comparison.eps", format='eps')
    plt.savefig('_'.join(filter(None, string.split('_')[1:])) + "_combined_names.eps", format='eps')
plt.close()
