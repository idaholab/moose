#!/usr/bin/env python3
# Script to run spatial convergence study on microwave_heating.i

import mms
import sympy
import os

if os.path.isfile('../../../../electromagnetics-opt'):
    executable = '../../../../electromagnetics-opt'
else:
     executable = '../../../../../combined/combined-opt'

df1 = mms.run_spatial('microwave_heating.i', 6, console=False, executable=executable, x_pp='h', y_pp=['error_real', 'error_imag','error_n'],mpi=1)

fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df1, label=['E_Real', 'E_Imag.','Heated Species'], marker='o', markersize=8)
fig.save('microwave_heating_convergence.png')
df1.to_csv('microwave_heating_convergence.csv')
