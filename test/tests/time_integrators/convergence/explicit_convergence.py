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

N = 6
EXE = '../../../../test'

df1 = mms.run_temporal('explicit_convergence.i', N,
                       'Executioner/TimeIntegrator/type=Heun', file_base='heun_{}',
                       console=False, dt=0.00390625, y_pp='l2_err', executable=EXE)

df2 = mms.run_temporal('explicit_convergence.i', N,
                       'Executioner/TimeIntegrator/type=Ralston', file_base='ralston_{}',
                       console=False, dt=0.00390625, y_pp='l2_err', executable=EXE)

df3 = mms.run_temporal('explicit_convergence.i', N,
                       'Executioner/TimeIntegrator/type=ExplicitMidpoint', file_base='explicitmidpoint_{}',
                       console=False, dt=0.00390625, y_pp='l2_err', executable=EXE)

df4 = mms.run_temporal('explicit_convergence.i', N,
                       'Executioner/TimeIntegrator/type=ExplicitEuler', file_base='expliciteuler_{}',
                       console=False, dt=0.00390625, y_pp='l2_err', executable=EXE)

fig = mms.ConvergencePlot(xlabel='$\Delta t$', ylabel='$L_2$ Error')
fig.plot(df1, label='Heun (2nd Order)', marker='o', markersize=8)
fig.plot(df2, label='Ralston (2nd Order)', marker='s', markersize=8)
fig.plot(df3, label='Midpoint (2nd Order)', marker='v', markersize=8)
fig.plot(df4, label='Euler (1st Order)', marker='h', markersize=8)
fig.save('explicit_convergence.pdf')
