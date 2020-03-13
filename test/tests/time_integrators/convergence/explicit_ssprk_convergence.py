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

def runConvergenceTest(input_file, additional_cli_args, dt, image_filename):
  df1 = mms.run_temporal(input_file, N,
                         'Executioner/TimeIntegrator/type=ExplicitEuler ' + additional_cli_args,
                         file_base='ee_{}', console=False, dt=dt, y_pp='l2_err', executable=EXE)

  df2 = mms.run_temporal(input_file, N,
                         'Executioner/TimeIntegrator/type=ActuallyExplicitEuler ' + additional_cli_args,
                         file_base='aee_{}', console=False, dt=dt, y_pp='l2_err', executable=EXE)

  df3 = mms.run_temporal(input_file, N,
                         'Executioner/TimeIntegrator/type=ExplicitSSPRungeKutta Executioner/TimeIntegrator/order=1 ' + additional_cli_args,
                         file_base='ssprk1_{}', console=False, dt=dt, y_pp='l2_err', executable=EXE)

  df4 = mms.run_temporal(input_file, N,
                         'Executioner/TimeIntegrator/type=ExplicitSSPRungeKutta Executioner/TimeIntegrator/order=2 ' + additional_cli_args,
                         file_base='ssprk2_{}', console=False, dt=dt, y_pp='l2_err', executable=EXE)

  df5 = mms.run_temporal(input_file, N,
                         'Executioner/TimeIntegrator/type=ExplicitSSPRungeKutta Executioner/TimeIntegrator/order=3 ' + additional_cli_args,
                         file_base='ssprk3_{}', console=False, dt=dt, y_pp='l2_err', executable=EXE)

  fig = mms.ConvergencePlot(xlabel='$\Delta t$', ylabel='$L_2$ Error')
  fig.plot(df1, label='ExplicitEuler', marker='+', markersize=8)
  fig.plot(df2, label='ActuallyExplicitEuler', marker='x', markersize=8)
  fig.plot(df3, label='SSP Runge-Kutta 1', marker='o', markersize=8, markerfacecolor='none')
  fig.plot(df4, label='SSP Runge-Kutta 2', marker='v', markersize=8)
  fig.plot(df5, label='SSP Runge-Kutta 3', marker='s', markersize=8)
  fig.save(image_filename)

additional_cli_args = 'Kernels/diff/implicit=true Kernels/ffn/implicit=true'
runConvergenceTest('explicit_convergence.i', additional_cli_args, 0.00390625, 'ssprk_convergence_with_bc.png')

additional_cli_args = 'Outputs/show=l2_err'
runConvergenceTest('../explicit_ssp_runge_kutta/explicit_ssp_runge_kutta.i', additional_cli_args, 0.1, 'ssprk_convergence_no_bc.png')
