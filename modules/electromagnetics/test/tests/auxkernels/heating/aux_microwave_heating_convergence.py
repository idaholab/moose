# Script to run spatial convergence study on aux_microwave_heating.i

import mms
import sympy


df1 = mms.run_spatial('aux_microwave_heating.i', 6, console=False, x_pp='h', y_pp=['error_real', 'error_imag','error_n','error_aux_heating'],mpi=1)

fig = mms.ConvergencePlot(xlabel='Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df1, label=['E_Real', 'E_Imag.','Heated Species','Aux Heating Term'], marker='o', markersize=8)
fig.save('aux_microwave_heating_convergence.png')
