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
import plotly
import plotly.graph_objects as go
import mooseutils

def command_line_options():
    """
    Command-line options for histogram tool.
    """
    parser = argparse.ArgumentParser(description="Command-line utility for creating histograms from VectorPostprocessor data.")
    parser.add_argument('filenames', nargs='+', type=str, help="The VectorPostprocessor data file pattern to open, for sample 'foo_x_*.csv'.")
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
    data = []
    for file in opt.filenames:
        data.append(mooseutils.PostprocessorReader(os.path.abspath(file)))

    # Set the default vector names
    vectors = []
    if not opt.vectors:
        vectors = data[0].variables()
    else:
        vectors = opt.vectors

    # Plot the results
    fig_data = [dict() for k in range(len(vectors)*len(data))]
    k = -1
    for i in range(len(data)):
        for j in range(len(vectors)):
            if not vectors[j] in data[i].variables():
                fig_data.pop(k+1)
                continue
            k += 1
            fig_data[k]['type'] = 'histogram'
            fig_data[k]['x'] = data[i][vectors[j]].tolist()
            fig_data[k]['nbinsx'] = opt.bins
            fig_data[k]['opacity'] = opt.alpha
            if opt.probability:
                fig_data[k]['histnorm'] = 'probability'
            if i < len(opt.names):
                fig_data[k]['name'] = opt.names[k]
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
