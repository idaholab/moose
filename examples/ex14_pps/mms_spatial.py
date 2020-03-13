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
df1 = mms.run_spatial('ex14.i', 4, executable='./ex14-opt')
df2 = mms.run_spatial('ex14.i', 4, 'Mesh/second_order=true', 'Variables/forced/order=SECOND', executable='./ex14-opt')

fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df1, label='1st Order', marker='o', markersize=8)
fig.plot(df2, label='2nd Order', marker='o', markersize=8)
fig.save('ex14_mms.png')
