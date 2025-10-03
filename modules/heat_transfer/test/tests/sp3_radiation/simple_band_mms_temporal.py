#!/usr/bin/env python3

import mms
df = mms.run_temporal('simple_band_mms.i', 4, console=False, dt=1e-5, executable='../../../heat_transfer-opt')
fig = mms.ConvergencePlot(xlabel=r'$\Delta$t', ylabel='$L_2$ Error')
fig.plot(df, label='1st Order (Implicit Euler)', marker='o', markersize=8)
fig.save('simple_band_mms_temporal.png')
df.to_csv('simple_band_mms_temporal.csv')
