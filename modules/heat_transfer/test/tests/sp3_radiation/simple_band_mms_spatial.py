#!/usr/bin/env python3

import mms
df = mms.run_spatial('simple_band_mms.i', 4, console=False, executable='../../../heat_transfer-opt')
fig = mms.ConvergencePlot(xlabel='Average Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df, label='1st Order', marker='o', markersize=8)
fig.save('simple_band_mms_spatial.png')
df.to_csv('simple_band_mms_spatial.csv')
