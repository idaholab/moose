import mms

# df_same = mms.run_spatial('same.i', 4, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')
# fig.plot(df_same, label=['u_same', 'lm_same'], num_fitted_points=3, slope_precision=1, marker='o')

# df_slave_coarser = mms.run_spatial('coarser-slave.i', 4, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig.plot(df_slave_coarser, label=['u_coarse_slave', 'lm_coarse_slave'], num_fitted_points=3, slope_precision=1, marker='o', linestyle='-.')

# df_slave_coarser_stab = mms.run_spatial('coarser-slave-delta-0.4.i', 4, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig.plot(df_slave_coarser, label=['u_coarse_slave_stab', 'lm_coarse_slave_stab'], num_fitted_points=3, slope_precision=1, marker='o', linestyle='--')

# df_master_coarser = mms.run_spatial('coarser-master.i', 4, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig.plot(df_master_coarser, label=['u_coarse_master', 'lm_coarse_master'], num_fitted_points=3, slope_precision=1, marker='o')

# df_master_coarser_stab = mms.run_spatial('coarser-master-delta-0.4.i', 4, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig.plot(df_master_coarser_stab, label=['u_coarse_master_stab', 'lm_coarse_master_stab'], title='moose-mesh-p2p1', num_fitted_points=3, slope_precision=1, marker='o')

# fig.save('multiple-lines.png')

# df_2_to_2 = mms.run_spatial('2-to-2-slave-to-master.i', 7, x_pp='h', y_pp=['L2u', 'L2lambda'])
# fig = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')
# fig.plot(df_2_to_2, label=['u_2_to_2', 'lm_2_to_2'], num_fitted_points=3, slope_precision=1, marker='o')

# fig.save('2-to-2.png')

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


def do_plots(fine_y, num_refinements, name=None):

    fig = mms.ConvergencePlot('Element Size ($h$)', ylabel='$L_2$ Error')

    do_plot('continuity.i',
            num_refinements,
            "Mesh/left_block/ny="+str(fine_y),
            fig,
            "coarse_master")
    do_plot('continuity.i',
            num_refinements,
            "Mesh/left_block/ny="+str(fine_y)+" Constraints/mortar/delta=0.4",
            fig,
            "coarse_master_stab")
    do_plot('continuity.i',
            num_refinements,
            "Mesh/right_block/ny="+str(fine_y),
            fig,
            "coarse_slave")
    do_plot('continuity.i',
            num_refinements,
            "Mesh/right_block/ny="+str(fine_y)+" Constraints/mortar/delta=0.4",
            fig,
            "coarse_slave_stab")

    if name:
        fig.set_title(name)

    if not name:
        name = 'ny-'+str(fine_y)
    fig.save(name+'.png')

do_plots(5, 5, 'p2p1-ny-5')
do_plots(4, 5, 'p2p1-ny-4')
