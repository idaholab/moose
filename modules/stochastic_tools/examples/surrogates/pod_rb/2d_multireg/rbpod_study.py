#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import os
import argparse
import numpy as np
import mooseutils
import csv
from scipy.integrate import quad

def command_line_options():
    """
    Command-line options for histogram tool.
    """
    parser = argparse.ArgumentParser(description="Run case study for RB-POD surrogate.")
    parser.add_argument('--mpicmd', default='mpiexec', type=str, help="MPI command, default is mpiexec.")
    parser.add_argument('--np', default=1, type=int, help="Number of processors to use for calculation, default is 1.")
    parser.add_argument('--runfom', default=1, type=int, help="Flag to run the full-order model for reference, default is True.")
    parser.add_argument('--runtrainer', default=1, type=int, help="Flag to run the trainer, default is True.")
    return parser.parse_args()

if __name__ == '__main__':

    # Command-line options
    opt = command_line_options()

    # Files created
    fom_results = 'full_order_out_results_0001.csv'
    rom_results = 'surr_out_res_0001.csv'

    # Commands
    train_cmd = opt.mpicmd + ' -n ' + str(opt.np) + ' ../../../../stochastic_tools-opt -i trainer.i --allow-test-objects'
    surr_cmd = opt.mpicmd + ' -n ' + str(opt.np) + ' ../../../../stochastic_tools-opt -i surr.i --allow-test-objects'
    fom_cmd = opt.mpicmd + ' -n ' + str(opt.np) + ' ../../../../stochastic_tools-opt -i full_order.i --allow-test-objects'

    no_pod_modes = [1, 2, 4, 8, 16, 32]

    if(opt.runtrainer):
        print('Obtaining Full-Order solutions for training (might be slow)')
        os.system(train_cmd + ' > tmp.txt')
        print('Training finished.')

    if(opt.runfom):
        print('Obtaining Full-Order solutions for testing (might be slow)')
        os.system(fom_cmd + ' > tmp.txt')
        print('Reference values obtained.')
        os.system('rm surr*0000.csv')
    fom_data = mooseutils.PostprocessorReader(fom_results)

    mean_rel_diff = []
    max_rel_diff = []

    for no_modes in no_pod_modes:
        cmd = surr_cmd + " Surrogates/rbpod/new_ranks=" + str(no_modes)
        print('Obtaining surrogate results with ', no_modes, ' POD modes.')
        os.system(cmd) # + ' > tmp.txt')
        print('Surrogate run finished.')

        os.system('rm surr*0000.csv')
        rom_data = mooseutils.PostprocessorReader(rom_results)
        os.system('mv surr_out_res_0001.csv surr_out_res_'+str(no_modes)+'.csv')

        rel_diff = abs(fom_data['results:nodal_l2'].values - rom_data['rbpod:nodal_l2'].values)/abs(fom_data['results:nodal_l2'].values)

        mean_rel_diff.append(rel_diff.mean())
        max_rel_diff.append(rel_diff.max())

    out_file = open("2d_multireg_results.csv", "w+")
    writer = csv.writer(out_file)
    writer.writerow(["no_modes", "mean", "max"])
    for i in range(len(mean_rel_diff)):
        writer.writerow([no_pod_modes[i], mean_rel_diff[i], max_rel_diff[i]])
