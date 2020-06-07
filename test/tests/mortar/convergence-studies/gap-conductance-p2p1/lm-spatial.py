import mms

# df_same = mms.run_spatial('same.i', 6, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')
# fig.plot(df_same, label=['u_same', 'lm_same'], num_fitted_points=3, slope_precision=1)

# df_slave_coarser = mms.run_spatial('coarser-slave.i', 6, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig.plot(df_slave_coarser, label=['u_coarse_slave', 'lm_coarse_slave'], num_fitted_points=3, slope_precision=1)

# df_master_coarser = mms.run_spatial('coarser-master.i', 6, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig.plot(df_master_coarser, label=['u_coarse_master', 'lm_coarse_master'], title='moose-mesh-p1p0', num_fitted_points=3, slope_precision=1)

# fig.save('multiple-lines.png')

# df_2_to_2 = mms.run_spatial('2-to-2-slave-to-master.i', 7, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')
# fig.plot(df_2_to_2, label=['u_2_to_2', 'lm_2_to_2'], num_fitted_points=3, slope_precision=1, marker='o')

# fig.save('2-to-2.png')

# df_4_to_2 = mms.run_spatial('4-to-2-slave-to-master.i', 7, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')
# fig.plot(df_4_to_2, label=['u_4_to_2', 'lm_4_to_2'], num_fitted_points=3, slope_precision=1, marker='o')

# df_4_to_2_stab = mms.run_spatial('4-to-2-slave-to-master-delta-0.4.i', 7, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig.plot(df_4_to_2_stab, label=['u_4_to_2_stab', 'lm_4_to_2_stab'], num_fitted_points=3, slope_precision=1, marker='o')

# fig.save('4-to-2.png')

def do_plot(input_file, num_refinements, cli_args, figure, label):
    df = mms.run_spatial(input_file,
                         num_refinements,
                         cli_args,
                         x_pp='h',
                         y_pp=['L2u', 'L2lambda'])
    figure.plot(df,
                label=['u_' + label, 'lm_' + label],
                num_fitted_points=3,
                slope_precision=1,
                marker='o')


def do_plots(fine, num_refinements, name=None):

    fig = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')

    do_plot('gap-conductance.i',
            num_refinements,
            "Mesh/left_block/ny="+str(fine)+" Mesh/left_block/nx="+str(fine),
            fig,
            "coarse_master")
    do_plot('gap-conductance.i',
            num_refinements,
            "Mesh/right_block/ny="+str(fine)+" Mesh/right_block/nx="+str(fine),
            fig,
            "coarse_slave")

    if name:
        fig.set_title(name)

    if not name:
        name = 'ny-'+str(fine)
    fig.save(name+'.png')

do_plots(1, 5, 'p2p1-1-to-3')
do_plots(2, 5, 'p2p1-2-to-3')
do_plots(3, 5, 'p2p1-3-to-3')
do_plots(4, 5, 'p2p1-4-to-3')
do_plots(5, 5, 'p2p1-5-to-3')
do_plots(6, 5, 'p2p1-6-to-3')
do_plots(7, 5, 'p2p1-7-to-3')
# do_plots(8, 5, 'p2p1-8-to-4')
# do_plots(9, 5, 'p2p1-9-to-4')
