import mms

df_same = mms.run_spatial('same.i', 5, x_pp='h', y_pp=['L2u', 'L2lambda'])
fig = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')
fig.plot(df_same, label=['u_same', 'lm_same'], num_fitted_points=3, slope_precision=1)

df_slave_coarser = mms.run_spatial('coarser-slave.i', 5, x_pp='h', y_pp=['L2u', 'L2lambda'])
fig.plot(df_slave_coarser, label=['u_coarse_slave', 'lm_coarse_slave'], num_fitted_points=3, slope_precision=1)

df_master_coarser = mms.run_spatial('coarser-master.i', 5, x_pp='h', y_pp=['L2u', 'L2lambda'])
fig.plot(df_master_coarser, label=['u_coarse_master', 'lm_coarse_master'], title='moose-mesh-p2p1', num_fitted_points=3, slope_precision=1)

fig.save('multiple-lines.png')
