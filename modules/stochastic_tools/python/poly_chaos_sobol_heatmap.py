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
    parser = argparse.ArgumentParser(description="Plot a 2D bar graph of sobol statistics from a polynomial chaos surrogate.")
    parser.add_argument('filename', type=str, help="The Postprocessor file with polynomial chaos sobol statistics")
    parser.add_argument('--vector', default=None, type=str, help="QoI vector name.")
    parser.add_argument('--names', default=[], nargs='+', type=str, help="Names to show on axes.")
    parser.add_argument('--output', default=None, type=str, help="File name to save figure")
    return parser.parse_args()

if __name__ == '__main__':

    # Command-line options
    opt = command_line_options()

    data = mooseutils.PostprocessorReader(opt.filename)
    if not 'i_1' in data:
        raise Exception('File must contain first order indices')
    if not 'i_2' in data:
        raise Exception('File must contain second order indices')

    if opt.vector is None:
        opt.vector = data.variables()[0]

    ndim = max(data['i_1']) + 1
    st = 0
    if 'i_T' in data:
        st += ndim
    sobol = np.zeros((ndim,ndim))
    for n in range(st,len(data[opt.vector])):
        i = data['i_1'][n]
        j = data['i_2'][n]
        if 'i_3' in data:
            if data['i_2'][n] != data['i_3']:
                break
        sobol[i,j] = data[opt.vector][n]
        sobol[j,i] = sobol[i,j]

    lab = []
    for n in range(ndim):
        if n < len(opt.names):
            lab.append(opt.names[n])
        else:
            lab.append('P<sub>' + str(n) + '</sub>')
    zmin = int(np.floor(np.amin(np.log10(sobol))))
    zmax = int(np.ceil(np.amax(np.log10(sobol))))
    ztick = np.linspace(zmin, zmax, zmax-zmin+1, dtype=int)
    ztick_lab = []
    for z in ztick:
        ztick_lab.append('10<sup>' + str(z) + '</sup>')
    colorbar = dict(tickvals=ztick, ticktext=ztick_lab)
    fig_data = dict(type='heatmap',z=np.log10(sobol),x=lab,y=lab, colorbar=colorbar)
    fig_layout = dict()
    fig = go.Figure(data=fig_data,layout=fig_layout)

    if opt.output is None:
        fig.show()
    else:
        plotly.io.write_image(fig, os.path.abspath(opt.output))
