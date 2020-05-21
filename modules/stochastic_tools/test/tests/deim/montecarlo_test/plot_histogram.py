#!/usr/bin/env python3
import numpy as np
import matplotlib.mlab as mlab
import matplotlib.pyplot as plt

QoIs = ["$U_{max}$", "$U_{min}$", "$U_{avg}$"]
x = np.loadtxt("master_out_results_0002.csv", skiprows=1, delimiter=",")
statistics = np.loadtxt("master_out_stats_0002.csv", skiprows=1, delimiter=",")
num_bins = 30

for qoi_index in range(len(QoIs)):
    n, bins, patches = plt.hist(x[:,qoi_index], num_bins, facecolor='k', alpha=0.48, rwidth=0.9, density=True)
    plt.xlabel(QoIs[qoi_index])

    mean = "{:.{}f}".format(statistics[0,qoi_index+1], 4)
    CI_upper = "{:.{}f}".format(statistics[1,qoi_index+1], 4)
    CI_lower = "{:.{}f}".format(statistics[2,qoi_index+1], 4)
    plt.title("$\mu=$"+mean+", 95% CI: ["+CI_lower+", "+CI_upper+"]")
    plt.ylabel("Density")
    plt.savefig("density"+str(qoi_index)+".png")
    plt.close()
