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
import sys
import argparse
import pandas as pd
import json
import numpy as np
import plotly
from plotly import subplots
import plotly.graph_objects as go
import mooseutils

def command_line_options():
    """
    Command-line options for statistics tool.
    """
    parser = argparse.ArgumentParser(description="Command-line utility for visualizing or tabulating sobol indices computed from the stochastic tools module.")

    parser.add_argument('--text-table', dest='format', action='store_const', const=0)
    parser.add_argument('--markdown-table', dest='format', action='store_const', const=1)
    parser.add_argument('--latex-table', dest='format', action='store_const', const=2)
    parser.add_argument('--bar-plot', dest='format', action='store_const', const=3)
    parser.add_argument('--heatmap', dest='format', action='store_const', const=4)

    parser.add_argument('filename', type=str, help="The Reporter data file to open, for sample 'foo_out.json' or 'foo_out_*.json'.")
    parser.add_argument('-obj', '--object', default=None, type=str, help="StatisticsReporter object to consider, by default the first object found is used.")
    parser.add_argument('-val', '--values', default=[], nargs='+', type=str, help="List of values to consider, by default all values are considered.")
    parser.add_argument('-s', '--stat', default='TOTAL', type=str, help="Type of sobol index to output. Options: 'TOTAL' (default), 'FIRST_ORDER', 'SECOND_ORDER'.")
    parser.add_argument('-ci', '--confidence-interval', dest='confidence_interval', default=None, nargs=2, type=str, help="Pair of confidence interval levels to use, by default the largest and smallest are used.")
    parser.add_argument('-t', '--time', default=None, type=float, help="Time to consider, by default the last time is used.")
    parser.add_argument('-n', '--names', default='{}', type=str, help="Map between the value name and display name in json format. Example: '{\"value1_name\" : \"Value One\", \"value2_name\" : \"Value Two\"}'")
    parser.add_argument('-p', '--param-names', nargs='+', default=[], type=str, help="List of names to display for the parameters.")
    parser.add_argument('-nf', '--number-format', default='.4g', type=str, help="The number format for tables.")
    parser.add_argument('-o', '--output', default=None, type=str, help="File name to save figure or table")
    parser.add_argument('--ignore-ci', action='store_true', help="Use this argument to ignore confidence intervals.")
    parser.add_argument('--log-scale', action='store_true', help="Use this argument to plot on logrithmic scale.")

    parser.set_defaults(ignore_ci=False, format=0, log_scale=False)
    return parser.parse_args()

def totalTable(data, num_form):
    tab = dict()
    ind_head = '$S_T$'
    if data['ci_levels'] is not None:
        ind_head += ' ({}%, {}%) CI'.format(data['ci_levels'][0] * 100, data['ci_levels'][1] * 100)
    tab[ind_head] = []
    for pname in data['param_names']:
        tab[pname] = []

    for vname in data['vector_names']:
        tab[ind_head].append(vname)
        for i, pname in enumerate(data['param_names']):
            if data['ci_levels'] is not None:
                tab[pname].append('{:{nf}} ({:{nf}}, {:{nf}})'.format(data[vname]['val'][i], data[vname]['ci_minus'][i], data[vname]['ci_plus'][i], nf=num_form))
            else:
                tab[pname].append('{:{nf}}'.format(data[vname]['val'][i], nf=num_form))

    return pd.DataFrame(tab)

def firstOrderTable(data, num_form):

    tab = totalTable(data, num_form)
    return tab.rename(columns={tab.columns[0] : tab.columns[0].replace('S_T', 'S_i')})

def secondOrderTable(data, num_form):

    tab = dict()
    if len(data['vector_names']) > 1:
        tab['Value'] = []
    ind_head = '$S_{i,j}$'
    if data['ci_levels'] is not None:
        ind_head += ' ({}%, {}%) CI'.format(data['ci_levels'][0] * 100, data['ci_levels'][1] * 100)
    tab[ind_head] = []
    for pname in data['param_names']:
        tab[pname] = []


    for vname in data['vector_names']:
        val = data[vname]
        for i, pnamei in enumerate(data['param_names']):
            tab[ind_head].append(pnamei)
            for j, pnamej in enumerate(data['param_names']):
                if j > i:
                    tab[pnamej].append('-')
                elif data['ci_levels'] is None:
                    tab[pnamej].append('{:{nf}}'.format(val['val'][i,j], nf=num_form))
                else:
                    tab[pnamej].append('{:{nf}} ({:{nf}}, {:{nf}})'.format(val['val'][i,j], val['ci_minus'][i,j], val['ci_plus'][i,j], nf=num_form))

            if len(data['vector_names']) > 1:
                if i > 0:
                    tab['Value'].append(' ')
                else:
                    tab['Value'].append(vname)


    return pd.DataFrame(tab)

def barPlot(data, stat, log_scale):

    bars = []
    for i, pnamei in enumerate(data['param_names']):
        for j, pnamej in enumerate(data['param_names']):
            if j > i:
                break;
            if stat == 'SECOND_ORDER':
                name = pnamei + ", " + pnamej
                ind = (i, j)
            elif j == i:
                name = pnamei
                ind = i
            else:
                continue

            y = [data[vname]['val'].item(ind) for vname in data['vector_names']]
            error_y = None
            if data['ci_levels'] is not None:
                error_y = dict(type='data')
                error_y['arrayminus'] = [data[vname]['val'].item(ind) - data[vname]['ci_minus'].item(ind) for vname in data['vector_names']]
                error_y['array'] = [data[vname]['ci_plus'].item(ind) - data[vname]['val'].item(ind) for vname in data['vector_names']]
            bars.append(go.Bar(name=name, x=data['vector_names'], y=y, error_y=error_y))

    if stat == 'TOTAL':
        ylabel = '$S_T$'
    elif stat == 'FIRST_ORDER':
        ylabel = '$S_i$'
    elif stat == 'SECOND_ORDER':
        ylabel = '$S_{i,j}$'

    layout=go.Layout(barmode='group', yaxis=dict(title=ylabel, type=('log' if log_scale else 'linear')))
    return go.Figure(data=bars, layout=layout)

def heatmap(data, stat, log_scale):

    vnames = data['vector_names']
    fig = plotly.subplots.make_subplots(rows=len(vnames), cols=1, subplot_titles=vnames, vertical_spacing=0.15/len(vnames))

    for i, vname in enumerate(vnames):
        row = (i // 2) + 1
        col = (i % 2) + 1

        z = data[vname]['val']
        if stat != 'SECOND_ORDER':
            z = np.diag(z)

        if log_scale:
            zmin = int(np.floor(np.amin(np.log10(z[z>0]))))
            zmax = int(np.ceil(np.amax(np.log10(z[z>0]))))
            ztick = np.linspace(zmin, zmax, zmax-zmin+1, dtype=int)
            ztick_lab = ['10<sup>{}</sup>'.format(zt) for zt in ztick]
            z[z<0] = np.nan
            z = np.log10(z)
        else:
            ztick = None
            ztick_lab = None
        if stat == 'TOTAL':
            ctitle = '$S_T$'
        elif stat == 'FIRST_ORDER':
            ctitle = '$S_i$'
        elif stat == 'SECOND_ORDER':
            ctitle = '$S_{i,j}$'
        colorbar = go.heatmap.ColorBar(title=dict(text=ctitle, side='right'), x=1, y=(i+0.5)/len(vnames), len=1/len(vnames), tickvals=ztick, ticktext=ztick_lab)

        fig.add_trace(go.Heatmap(x=data['param_names'], y=data['param_names'], z=z, colorbar=colorbar), row=i+1, col=1)

    return fig

def main():

    # Command-line options
    opt = command_line_options()

    # Make sure stat is uppercase
    opt.stat = opt.stat.upper()
    # Define value_name-display_name map
    opt.names = json.loads(opt.names)

    # Read reporter data
    data = mooseutils.ReporterReader(opt.filename)
    if opt.time is not None:
        data.update(opt.time)
    if opt.object is None:
        opt.object = next(var[0] for var in data.variables() if data.info(var[0])['type'] == 'SobolReporter')
    repinfo = data.info(opt.object)
    if not len(opt.values):
        opt.values = [var[1] for var in data.variables() if var[0] == opt.object]

    # Dictionary for data storage
    frame = dict(param_names=[], ci_levels=None, vector_names=[])
    frame['param_names'] = [(opt.param_names[i] if i < len(opt.param_names) else str(i)) for i in range(repinfo['num_params'])]
    frame['indices'] = repinfo['indices']
    levels = False
    if not opt.ignore_ci and 'confidence_intervals' in repinfo:
        levels = repinfo['confidence_intervals']['levels']
        frame['ci_levels'] = (min(levels), max(levels)) if opt.confidence_interval is None else tuple(opt.confidence_interval)
    for val in opt.values:
        vn = opt.names[val] if val in opt.names else val
        rep = data[(opt.object, val)]
        frame['vector_names'].append(vn)
        repv = data[(opt.object, val)][opt.stat]
        frame[vn] = dict()
        frame[vn]['val'] = np.array(repv[0])
        if levels:
            frame[vn]['ci_minus'] = np.array(repv[1][levels.index(frame['ci_levels'][0])])
            frame[vn]['ci_plus'] = np.array(repv[1][levels.index(frame['ci_levels'][1])])

    # Tables
    if opt.format < 3:

        if opt.stat == 'TOTAL':
            tab = totalTable(frame, opt.number_format)
        elif opt.stat == 'FIRST_ORDER':
            tab = firstOrderTable(frame, opt.number_format)
        elif opt.stat == 'SECOND_ORDER':
            tab = secondOrderTable(frame, opt.number_format)

        if opt.format == 0:
            out = tab.to_string(index=False)
        elif opt.format == 1:
            out = tab.to_markdown(index=False)
        elif opt.format == 2:
            out = tab.to_latex(index=False)

        if opt.output is None:
            print(out)
        else:
            with open(os.path.abspath(opt.output), "w") as fid:
                fid.write(out)


    elif opt.format >= 3:

        if opt.format == 3:
            fig = barPlot(frame, opt.stat, opt.log_scale)
        elif opt.format == 4:
            fig = heatmap(frame, opt.stat, opt.log_scale)

        if opt.output is None:
            fig.show()
        else:
            plotly.io.write_image(fig, os.path.abspath(opt.output))

    return 0

if __name__ == '__main__':
    sys.exit(main())
