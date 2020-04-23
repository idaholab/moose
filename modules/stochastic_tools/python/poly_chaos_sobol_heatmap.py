#!/usr/bin/env python3
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import argparse
import numpy as np
import plotly.graph_objects as go
import mooseutils

def command_line_options():
    """
    Command-line options for histogram tool.
    """
    parser = argparse.ArgumentParser(description="Plot a 2D bar graph of sobol statistics from a polynomial chaos surrogate.")
    parser.add_argument('filename', type=str, help="The Postprocessor file with polynomial chaos sobol statistics")
    return parser.parse_args()

if __name__ == '__main__':

    # Command-line options
    opt = command_line_options()

    data = mooseutils.PostprocessorReader(opt.filename)
    if not 'i_1' in data:
        raise Exception('File must contain first order indices')
    if not 'i_2' in data:
        raise Exception('File must contain second order indices')

    ndim = max(data['i_1']) + 1
    st = 0
    if 'i_T' in data:
        st += ndim
    sobol = np.zeros((ndim,ndim))
    for n in range(st,len(data['value'])):
        i = data['i_1'][n]
        j = data['i_2'][n]
        if 'i_3' in data:
            if data['i_2'][n] != data['i_3']:
                break
        sobol[i,j] = data['value'][n]
        sobol[j,i] = sobol[i,j]

    lab = ['k', 'q', 'L', 'T<sub>&#8734;</sub>']
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
    fig.show()
