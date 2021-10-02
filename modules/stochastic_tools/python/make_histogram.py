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
import glob
import plotly
import plotly.graph_objects as go
import mooseutils

def command_line_options():
    """
    Command-line options for histogram tool.
    """
    parser = argparse.ArgumentParser(description="Command-line utility for creating histograms from VectorPostprocessor or Reporter data.")
    parser.add_argument('filenames', nargs='+', type=str, help="The VectorPostprocessor or Reporter data file to open. Can be pattern for distributed data 'large_data.json.*'")
    parser.add_argument('-v', '--vectors', default=[], nargs='+', type=str, help="List of vector names to consider, by default all vectors are shown.")
    parser.add_argument('--names', default=[], nargs='+', type=str, help="Name to show on legend, default is no legend.")
    parser.add_argument('--probability', default=True, type=bool, help="True to plot with probability density normalization.")
    parser.add_argument('--bins', default=None, type=int, help="Number of bins to consider.")
    parser.add_argument('--alpha', default=0.75, type=float, help="Set the bar chart opacity alpha setting.")
    parser.add_argument('--xlabel', default='Value', type=str, help="The X-axis label.")
    parser.add_argument('--ylabel', default='Probability', type=str, help="The Y-axis label.")
    parser.add_argument('--title', default=None, type=str, help="Title of figure.")
    parser.add_argument('--output', default=None, type=str, help="File name to save figure")
    return parser.parse_args()

if __name__ == '__main__':

    # Command-line options
    opt = command_line_options()

    # Read data
    data = dict()
    for file in opt.filenames:
        for fname in sorted(glob.glob(file)):

            if '.csv' in fname:
                vdata = mooseutils.PostprocessorReader(fname)
                vars = vdata.variables()
                ind = dict()
                for vec in vars:
                    obj_val = vec.split(':')
                    if obj_val[-1] == 'converged':
                        ind[obj_val[0]] = np.where(vdata[vec])[0]
                for vec in vars:
                    obj_val = vec.split(':')
                    if obj_val[-1] == 'converged':
                        continue
                    val = vdata[vec][ind[obj_val[0]]].tolist() if obj_val[0] in ind else vdata[vec].tolist()
                    if vec in data:
                        data[vec].extend(val)
                    else:
                        data[vec] = val

            else:
                rdata = mooseutils.ReporterReader(fname)
                vars = rdata.variables()
                ind = dict()
                for vec in vars:
                    obj_val = vec[1].split(':')
                    if obj_val[-1] == 'converged':
                        ind[obj_val[0]] = np.where(rdata[vec])[0]
                for vec in vars:
                    obj_val = vec[1].split(':')
                    if obj_val[-1] == 'converged' or rdata.info(vec)['type'] != 'std::vector<double>':
                        continue
                    val = np.array(rdata[vec])[ind[obj_val[0]]].tolist() if obj_val[0] in ind else rdata[vec]
                    if vec[1] in data:
                        data[vec[1]].extend(val)
                    else:
                        data[vec[1]] = val


    # Plot the results
    fig_data = []
    k = -1
    for vec in data:
        if len(opt.vectors) and not vec in opt.vectors:
            continue
        k = k + 1
        fig_frame = dict()
        fig_frame['type'] = 'histogram'
        fig_frame['x'] = data[vec]
        fig_frame['nbinsx'] = opt.bins
        fig_frame['opacity'] = opt.alpha
        if opt.probability:
            fig_frame['histnorm'] = 'probability'
        if k < len(opt.names):
            fig_frame['name'] = opt.names[k]
        fig_data.append(fig_frame)
    xaxis = dict()
    xaxis['title'] = opt.xlabel
    yaxis = dict()
    yaxis['title'] = opt.ylabel
    layout = dict()
    layout['xaxis'] = xaxis
    layout['yaxis'] = yaxis
    layout['showlegend'] = len(opt.names) > 0
    layout['barmode'] = 'overlay'
    layout['title'] = opt.title

    fig = go.Figure(data=fig_data, layout=layout)

    if opt.output is None:
        fig.show()
    else:
        plotly.io.write_image(fig, os.path.abspath(opt.output))
