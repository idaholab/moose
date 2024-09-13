# Script to run spatial convergence study on vector_conductive_current.i

import mms
import sympy

import mms
df1 = mms.run_spatial('vector_conductive_current.i', 6, console=False, x_pp='h', y_pp=['error_real', 'error_imag'],mpi=1)

fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df1, label=['E_Real', 'E_Imag.'], marker='o', markersize=8)
fig.save('vector_conductive_current_convergence.png')