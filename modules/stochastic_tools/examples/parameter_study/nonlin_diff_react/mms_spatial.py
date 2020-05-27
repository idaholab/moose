#!/usr/bin/env python3

import mms
from mms import ConvergencePlot

df1 = mms.run_spatial('nonlin_diff_react_mms.i', 4, '--allow-test-objects', console=False, executable='../../../stochastic_tools-opt')

fig = ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df1, label='1st Order', marker='o', markersize=8)
fig.save('mms_spatial.png')
