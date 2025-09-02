#!/usr/bin/env python

import mms
df1 = mms.run_temporal('temporal_test.i', 4, dt = 0.1, console=False, executable='../../../moose_test-opt', y_pp = ['error_u','error_v'])
df2 = mms.run_temporal('temporal_test.i', 4, 'Executioner/scheme=bdf2', dt = 0.1,
                      console=False, executable='../../../moose_test-opt', y_pp = ['error_u','error_v'])

fig = mms.ConvergencePlot(xlabel=r'$\Delta$t', ylabel='$L_2$ Error')
fig.plot(df1, label = ['error_u 1st Order (Implicit Euler)','error_v 1st Order (Implicit Euler)'], marker='o', markersize=8)
fig.plot(df2, label=  ['error_u 2nd Order (BDF2)','error_v 2nd Order (BDF2)'], marker='o', markersize=8)
fig.save('mms_temporal.png')

df1.to_csv('mms_temporal_first.csv')
df2.to_csv('mms_temporal_second.csv')
