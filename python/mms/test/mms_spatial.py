#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import mms
df1 = mms.run_spatial('mms_spatial.i', 4, console=False, executable='../../../test')
df2 = mms.run_spatial('mms_spatial.i', 4, 'Mesh/second_order=true', 'Variables/u/order=SECOND',
                      console=False, executable='../../../test')

fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df1, label='1st Order', marker='o', markersize=8)
fig.plot(df2, label='2nd Order', marker='o', markersize=8)
fig.save('mms_spatial.png')

#TESTING (leave this comment, it is used in doco to remove the following from a demo)
df1.to_csv('mms_spatial_first.csv')
df2.to_csv('mms_spatial_second.csv')
