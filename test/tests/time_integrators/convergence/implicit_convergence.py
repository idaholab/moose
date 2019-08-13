#!/usr/bin/env python2
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html
import mms

N = 3
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

if hasattr(mms, 'ConvergencePlot'):
    # The ConvergencePlot object requires matplotlib, but it is not always installed on the test
    # machines. The mms module checks for matplotlib before importing the ConvergencePlot. This
    # check allows this script to work with and without matplotlib.
    fig = mms.ConvergencePlot(xlabel='$\Delta t$', ylabel='$L_2$ Error')
    fig.plot(df1, label='Midpoint', marker='o', markersize=8)
    fig.plot(df2, label='L-stable DIRK3', marker='v', markersize=8)
    fig.plot(df3, label='L-stable DIRK4', marker='p', markersize=8)
    fig.plot(df4, label='A-stable DIRK4', marker='s', markersize=8)
    fig.plot(df5, label='A-stable DIRK4 (bootstrap)', marker='D', markersize=8)
    fig.save('implicit_convergence.pdf')

df1.to_csv('implicit_convergence_midpoint.csv')
df2.to_csv('implicit_convergence_lstabledirk3.csv')
df3.to_csv('implicit_convergence_lstabledirk4.csv')
df4.to_csv('implicit_convergence_astabledirk4.csv')
df5.to_csv('implicit_convergence_astabledirk4_bootstrap.csv')
