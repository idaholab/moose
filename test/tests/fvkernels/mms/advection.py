#!/usr/bin/env python3

import mms
df1 = mms.run_spatial('advection.i', 7)

fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df1, label='adv', marker='o', markersize=8, num_fitted_points=3, slope_precision=1)
fig.save('advection.png')
