#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

# This script generates pictures that demonstrate the agreement between the analytic LaForce solutions and MOOSE

import os
import sys
import numpy as np
import matplotlib.pyplot as plt

def moose(fn):
    try:
        #f = open(os.path.join("gold", fn))
        f = open(fn)
        data = f.readlines()[1:-1]
        data = [list(map(float, d.strip().split(","))) for d in data]
        log10_max_val = np.log10(len(data) - 1)
        num_pts_displayed = 150
        subsample = [np.power(10, log10_max_val * i / float(num_pts_displayed - 1)) for i in range(num_pts_displayed)] # 1 to max_val in logarithmic progression
        subsample = sorted(list(set([0] + [int(np.round(s)) for s in subsample])))  # 0 to len(data)-1 in log progression
        data = [data[i] for i in subsample]
        data = ([d[9] for d in data], [d[5] for d in data], [d[3] for d in data])
        f.close()
    except:
        sys.stderr.write("Cannot read " + fn + ", or it contains erroneous data\n")
        sys.exit(1)
    return data


moose_timesteps = ["0062", "0098", "0135", "0231"]
moose_timesteps = ["0078", "0204", "0412", "0745"]
moose_timesteps = ["0078", "0179", "0386", "0662"]
moosePTSUSS = [moose("2D_c_csv_ptsuss_" + ts + ".csv") for ts in moose_timesteps]

plt.figure()
plt.semilogx(moosePTSUSS[1][0], moosePTSUSS[1][1], 'r-', label = 'S$_{\mathrm{gas}}$ (1 day)')
plt.semilogx(moosePTSUSS[1][0], moosePTSUSS[1][2], 'r:', label = 'Porosity (1 day)')
plt.semilogx(moosePTSUSS[3][0], moosePTSUSS[3][1], 'k-', label = 'S$_{\mathrm{gas}}$ (5 years)')
plt.semilogx(moosePTSUSS[3][0], moosePTSUSS[3][2], 'k:', label = 'Porosity (5 years)')
plt.legend(loc = 'best', prop = {'size': 10})
plt.xlim([0.1, 5000])
plt.xlabel("r (m)")
plt.ylabel("S$_{\mathrm{gas}}$ and Porosity")
plt.title("With reservoir dissolution and porosity changes")
plt.savefig("../../doc/content/media/porous_flow/2D_c_fig.png")
plt.show()

sys.exit(0)
