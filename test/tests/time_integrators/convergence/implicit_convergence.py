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

N = 5
EXE = '../../../../test'

df1 = mms.run_temporal('implicit_convergence.i', N,
                       'Executioner/TimeIntegrator/type=ImplicitMidpoint',
                       file_base='implicitmidpoint_{}',
                       console=False, dt=0.0625, y_pp='l2_err', executable=EXE)

df2 = mms.run_temporal('implicit_convergence.i', N,
                       'Executioner/TimeIntegrator/type=LStableDirk3',
                       file_base='lstabledirk3_{}',
                       console=False, dt=0.0625, y_pp='l2_err', executable=EXE)

df3 = mms.run_temporal('implicit_convergence.i', N,
                       'Executioner/TimeIntegrator/type=LStableDirk4',
                       file_base='lstabledirk4_{}',
                       console=False, dt=0.5, y_pp='l2_err', executable=EXE)

df4 = mms.run_temporal('implicit_convergence.i', N,
                       'Executioner/TimeIntegrator/type=AStableDirk4',
                       'Executioner/TimeIntegrator/safe_start=False',
                       file_base='astabledirk4_{}',
                       console=False, dt=0.0625, y_pp='l2_err', executable=EXE)

df5 = mms.run_temporal('implicit_convergence.i', N,
                       'Executioner/TimeIntegrator/type=AStableDirk4',
                       file_base='astabledirk4_bootstrap_{}',
                       console=False, dt=0.5, y_pp='l2_err', executable=EXE)

fig = mms.ConvergencePlot(xlabel='$\Delta t$', ylabel='$L_2$ Error')
fig.plot(df1, label='Midpoint', marker='o', markersize=8)
fig.plot(df2, label='L-stable DIRK3', marker='v', markersize=8)
fig.plot(df3, label='L-stable DIRK4', marker='p', markersize=8)
fig.plot(df4, label='A-stable DIRK4', marker='s', markersize=8)
fig.plot(df5, label='A-stable DIRK4 (bootstrap)', marker='D', markersize=8)
fig.save('implicit_convergence.pdf')
