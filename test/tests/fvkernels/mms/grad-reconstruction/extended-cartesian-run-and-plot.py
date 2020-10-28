#!/usr/bin/env python3

import mms
df1 = mms.run_spatial('extended-cartesian.i', 7, mpi=2)

fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df1, label='extended-cartesian', marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
fig.save('extended-cartesian.png')
