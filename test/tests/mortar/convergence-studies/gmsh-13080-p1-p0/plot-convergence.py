import mms

df_p1p0_coarse_master = mms.run_spatial('p1p0-coarser-master.i', 6, x_pp='h', y_pp=['L2u', 'L2lambda'])
fig_p1p0 = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')
fig_p1p0.plot(df_p1p0_coarse_master, label=['u_coarse_master', 'lm_coarse_master'], title='p1p0', num_fitted_points=3, slope_precision=1)

df_p1p0_coarse_slave = mms.run_spatial('p1p0-coarser-slave.i', 6, x_pp='h', y_pp=['L2u', 'L2lambda'])
fig_p1p0.plot(df_p1p0_coarse_slave, label=['u_coarse_slave', 'lm_coarse_slave'], title='p1p0', num_fitted_points=3, slope_precision=1)
fig_p1p0.save('p1p0-multiple-lines.png')
