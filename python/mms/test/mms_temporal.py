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
df1 = mms.run_temporal('mms_temporal.i', 4, console=False, executable='../../../test')
df2 = mms.run_temporal('mms_temporal.i', 4, 'Executioner/scheme=bdf2', console=False,
                       executable='../../../test')

fig = mms.ConvergencePlot(xlabel=r'$\Delta$t', ylabel='$L_2$ Error')
fig.plot(df1, label='1st Order (Implicit Euler)', marker='o', markersize=8)
fig.plot(df2, label='2nd Order (Backward Difference)', marker='o', markersize=8)
fig.save('mms_temporal.png')

#TESTING (leave this comment, it is used in doco to remove the following from a demo)
df1.to_csv('mms_temporal_first.csv')
df2.to_csv('mms_temporal_second.csv')
