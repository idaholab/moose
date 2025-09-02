#!/usr/bin/env python

import mms
df1 = mms.run_spatial('spatial_test.i', 4, console=False, executable='../../../moose_test-opt', y_pp = ['error_u','error_v'])
df2 = mms.run_spatial('spatial_test.i', 4, 'Mesh/second_order=' \
'true', 'Variables/v/order=SECOND', 'Variables/u/order=SECOND', console=False, executable='../../../moose_test-opt', y_pp = ['error_u','error_v'])

fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df1, label = ['error_u 1st Order','error_v 1st Order'], marker='o', markersize=8)
fig.plot(df2, label=  ['error_u 2nd Order','error_v 2nd order'], marker='o', markersize=8)
fig.save('mms_spatial.png')

df1.to_csv('mms_spatial_first.csv')
df2.to_csv('mms_spatial_second.csv')
