import mms

df_p2p1_coarse_master = mms.run_spatial('p2p1-coarser-master.i', 5, x_pp='h', y_pp=['L2u', 'L2lambda'])
fig_p2p1 = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')
fig_p2p1.plot(df_p2p1_coarse_master, label=['u_coarse_master', 'lm_coarse_master'], title='p2p1', num_fitted_points=3, slope_precision=1)

df_p2p1_coarse_slave = mms.run_spatial('p2p1-coarser-slave.i', 5, x_pp='h', y_pp=['L2u', 'L2lambda'])
fig_p2p1.plot(df_p2p1_coarse_slave, label=['u_coarse_slave', 'lm_coarse_slave'], title='p2p1', num_fitted_points=3, slope_precision=1)
fig_p2p1.save('p2p1-multiple-lines.png')
