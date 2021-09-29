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
import plotly.graph_objects as go
import mooseutils

def command_line_options():
    """
    Command-line options for statistics tool.
    """
    parser = argparse.ArgumentParser(description="Command-line utility for visualizing or tabulating statistics computed from the stochastic tools module.")

    parser.add_argument('--text-table', dest='format', action='store_const', const=0)
    parser.add_argument('--markdown-table', dest='format', action='store_const', const=1)
    parser.add_argument('--latex-table', dest='format', action='store_const', const=2)
    parser.add_argument('--bar-plot', dest='format', action='store_const', const=3)
    parser.add_argument('--line-plot', dest='format', action='store_const', const=4)

    parser.add_argument('filenames', nargs='+', type=str, help="The Reporter data file to open, for sample 'foo_out.json ' or 'foo_out_*.json'.")
    parser.add_argument('-obj', '--objects', default=[], nargs='+', type=str, help="List of StatisticsReporter objects to consider, by default all objects are considered.")
    parser.add_argument('-val', '--values', default=[], nargs='+', type=str, help="List of values to consider, by default all values are considered.")
    parser.add_argument('-s', '--stats', default=[], nargs='+', type=str, help="List of stats to consider, by default all stats are considered.")
    parser.add_argument('-ci', '--confidence-interval', dest='confidence_interval', default=None, nargs=2, type=float, help="Pair of confidence interval levels to use, by default the largest and smallest are used.")
    parser.add_argument('-t', '--times', default=[], nargs='+', type=float, help="List of times to consider, by all times are considered.")
    parser.add_argument('-n', '--names', default='{}', type=str, help="Map between the value name and display name in json format. Example: '{\"value1_name\" : \"Value One\", \"value2_name\" : \"Value Two\"}'")
    parser.add_argument('-sn', '--stat-names', default='{}', type=str, help="Map between the statistic name and  what is displayed in json format. Example: '{\"MEAN\" : \"Average\", \"STDDEV\" : \"Standard Deviation\"}'")
    parser.add_argument('-nf', '--number-format', default='.4g', type=str, help="The number format for tables.")
    parser.add_argument('-o', '--output', default=None, type=str, help="File name to save figure or table")
    parser.add_argument('--ignore-ci', action='store_true', help="Use this argument to ignore confidence intervals.")

    parser.add_argument('--xvalue', default='Time', type=str, help="The value to use for x value in the line plot. Default is time. This is only relevant with '--line-plot'.")
    parser.add_argument('--xlabel', default=None, type=str, help="The x-axis label in the line plot. Default is whatever '--xvalue' is. This is only relevant with '--line-plot'.")
    parser.set_defaults(ignore_ci=False, format=0)
    return parser.parse_args()

def statTable(data, num_form):

    tab = dict(Values = [])
    times = data.time.unique()
    stats = data.statistic.unique()
    values = data.vector_name.unique()
    if len(times) > 1:
        tab['Time'] = []
    for stat in stats:
        tab[stat] = []

    for vname in values:
        row = data[(data.vector_name == vname)].iloc(0)[0]
        if isinstance(row['ci_levels'], tuple):
            tab['Values'].append('{} ({}%, {}%) CI'.format(row['vector_name'], row['ci_levels'][0] * 100, row['ci_levels'][1] * 100))
        else:
            tab['Values'].append(row['vector_name'])
        if len(times) > 1:
            for step, time in enumerate(times):
                tab['Time'].append('{:{nf}}'.format(time, nf=num_form))
                if step > 0:
                    tab['Values'].append(' ')

    for stat in stats:
        for vname in values:
            for time in times:
                row = data[(data.vector_name == vname) & (data.time == time) & (data.statistic == stat)].iloc(0)[0]
                if isinstance(row['ci_levels'], tuple):
                    tab[stat].append('{:{nf}} ({:{nf}}, {:{nf}})'.format(row['value'], row['confidence_interval'][0], row['confidence_interval'][1], nf=num_form))
                else:
                    tab[stat].append('{:{nf}}'.format(row['value'], nf=num_form))

    return pd.DataFrame(tab)

def statBar(data):
    stats = data.statistic.unique()
    vecs = data.vector_name.unique()
    time = data.time.max()
    bars = []
    for stat in stats:
        df = data[(data.statistic == stat) & (data.time == time)]
        vnames= []
        ciminus = []
        ciplus = []
        for index, row in df.iterrows():
            if isinstance(row['ci_levels'], tuple):
                vnames.append('{} ({}%, {}%) CI'.format(row['vector_name'], row['ci_levels'][0] * 100, row['ci_levels'][1] * 100))
                ciminus.append(row['value'] - row['confidence_interval'][0])
                ciplus.append(row['confidence_interval'][1] - row['value'])
            else:
                vnames.append(row['vector_name'])
                ciminus.append(0)
                ciplus.append(0)

        bars.append(go.Bar(name=stat, x=vnames, y=df.value, error_y=dict(type='data', array=ciplus, arrayminus=ciminus)))
    layout = go.Layout(barmode='group')
    return go.Figure(data=bars, layout=layout)

def statTimeLine(data, xlabel):
    stats = data.statistic.unique()
    vecs = data.vector_name.unique()
    lines = []

    for vec in vecs:
        for stat in stats:
            df = data[(data.statistic == stat) & (data.vector_name == vec)].sort_values(by=['time'])
            ci_df = df[df.ci_levels.notna()]
            line = go.Scatter(x=df.time, y=df.value)

            name = []
            if len(vecs) > 1:
                name.append(vec)
            if len(stats) > 1:
                name.append(stat)
            if len(ci_df) and len(vecs) > 1:
                name.append('({}%, {}%) CI'.format(ci_df.ci_levels.iloc[0][0] * 100, ci_df.ci_levels.iloc[0][1] * 100))
            if len(name):
                line['name'] = ' '.join(name)

            if len(ci_df):
                ciminus = []
                ciplus = []
                for index, row in df.iterrows():
                    if isinstance(row['ci_levels'], tuple):
                        ciminus.append(row['value'] - row['confidence_interval'][0])
                        ciplus.append(row['confidence_interval'][1] - row['value'])
                    else:
                        ciminus.append(0)
                        ciplus.append(0)
                line['error_y'] = dict(type='data', array=ciplus, arrayminus=ciminus)

            lines.append(line)

    ylabel = []
    if len(vecs) == 1:
        ylabel.append(vecs[0])
    if len(stats) == 1:
        ylabel.append(stats[0])
    if len(data[data.ci_levels != None]) and len(vecs) == 1:
        ylabel.append('({}%, {}%) CI'.format(data[data.ci_levels != None].ci_levels.iloc[0][0] * 100, data[data.ci_levels != None].ci_levels.iloc[0][1] * 100))
    ylabel = ' '.join(ylabel) if len(ylabel) else 'Value'

    return go.Figure(data=lines, layout=go.Layout(xaxis=dict(title=xlabel), yaxis=dict(title=ylabel)))

def statLine(data, xname):
    xdata = data[data.vector_name == xname]
    ydata = data[data.vector_name != xname]
    stats = ydata.statistic.unique()
    vecs = ydata.vector_name.unique()
    lines = []

    for vec in vecs:
        for stat in stats:
            df = ydata[(ydata.vector_name == vec) & (ydata.statistic == stat)].sort_values(by=['time'])
            for time in df.time:
                yrow = df[df.time == time].iloc(0)[0]
                xrow = xdata[(xdata.statistic == stat) & (xdata.time == time)].iloc(0)[0]

                line = go.Scatter(x=xrow['value'], y=yrow['value'])

                name = []
                if len(vecs) > 1:
                    name.append(vec)
                name.append(stat)
                if  isinstance(yrow['confidence_interval'], tuple) and len(vecs) > 1:
                    name.append('({}%, {}%) CI'.format(ci_df.ci_levels.iloc[0][0] * 100, ci_df.ci_levels.iloc[0][1] * 100))
                if len(df.time) > 1:
                    name.append('Time = {}'.format(time))
                if len(name):
                    line['name'] = ' '.join(name)

                if isinstance(yrow['confidence_interval'], tuple):
                    ciminus = np.asarray(yrow['value']) - np.asarray(yrow['confidence_interval'][0])
                    ciplus = np.asarray(yrow['confidence_interval'][1]) - np.asarray(yrow['value'])
                    line['error_y'] = dict(type='data', array=ciplus, arrayminus=ciminus)

                if isinstance(xrow['confidence_interval'], tuple):
                    ciminus = np.asarray(xrow['value']) - np.asarray(xrow['confidence_interval'][0])
                    ciplus = np.asarray(xrow['confidence_interval'][1]) - np.asarray(xrow['value'])
                    line['error_x'] = dict(type='data', array=ciplus, arrayminus=ciminus)

                lines.append(line)

    ylabel = []
    if len(vecs) == 1:
        ylabel.append(vecs[0])
    y_ci_level = ydata[ydata.ci_levels.notna()].ci_levels
    if len(y_ci_level) and len(vecs) == 1:
        ylabel.append('({}%, {}%) CI'.format(y_ci_level.iloc[0][0] * 100, y_ci_level.iloc[0][1] * 100))
    ylabel = ' '.join(ylabel) if len(ylabel) else 'Value'

    xlabel = xname
    x_ci_level = xdata[xdata.ci_levels.notna()].ci_levels
    if len(xdata[xdata.ci_levels.notna()]):
        xlabel += ' ({}%, {}%) CI'.format(x_ci_level.iloc[0][0] * 100, x_ci_level.iloc[0][1] * 100)

    return go.Figure(data=lines, layout=go.Layout(xaxis=dict(title=xlabel), yaxis=dict(title=ylabel), showlegend=True))

def textTable(data, num_form, file):
    tab = statTable(data, num_form)
    if file is None:
        print(tab.to_string(index=False))
    else:
        with open(os.path.abspath(file), "w") as fid:
            fid.write(tab.to_string(index=False))

def markdownTable(data, num_form, file):
    tab = statTable(data, num_form)
    if file is None:
        print(tab.to_markdown(index=False))
    else:
        with open(os.path.abspath(file), "w") as fid:
            fid.write(tab.to_markdown(index=False))

def latexTable(data, num_form, file):
    tab = statTable(data, num_form)
    if file is None:
        print(tab.to_latex(index=False))
    else:
        with open(os.path.abspath(file), "w") as fid:
            fid.write(tab.to_latex(index=False))

def barPlot(data, file):
    fig = statBar(data)
    if file is None:
        fig.show()
    else:
        plotly.io.write_image(fig, os.path.abspath(file))

def linePlot(data, xname, file):
    if xname in data.vector_name.unique():
        fig = statLine(data, xname)
    else:
        fig = statTimeLine(data, xname)
    if file is None:
        fig.show()
    else:
        plotly.io.write_image(fig, os.path.abspath(file))

def main():

    # Command-line options
    opt = command_line_options()

    # Convert stats to uppercase
    opt.stats = [stat.upper() for stat in opt.stats]

    # Define value_name-display_name map
    opt.names = json.loads(opt.names)
    opt.stat_names = dict((key.upper(), value) for (key, value) in json.loads(opt.stat_names).items())

    # Grab ALL the data
    frame = dict(time=[], object_name=[], vector_name=[], statistic=[], ci_levels=[], value=[], confidence_interval=[])
    for file in opt.filenames:
        data = mooseutils.ReporterReader(file)

        times = opt.times if len(opt.times) else data.times()
        for time in times:
            data.update(time)
            for var in data.variables():
                # Object name
                objname = var[0]
                if (len(opt.objects) and objname not in opt.objects) or data.info(objname)['type'] != 'StatisticsReporter':
                    continue

                # Split value name into vector name and stat
                tmp = var[1].split('_')
                vecname = '_'.join(tmp[:-1])
                if len(opt.values) and vecname not in opt.values:
                    continue
                stat = tmp[-1]
                if len(opt.stats) and stat not in opt.stats:
                    continue

                # Checks are done so start pushing back
                frame['time'].append(time)
                frame['object_name'].append(objname)
                frame['vector_name'].append(opt.names[vecname] if vecname in opt.names else vecname)
                frame['statistic'].append(opt.stat_names[stat] if stat in opt.stat_names else stat)

                # Grab value and confidence interval
                val = data[var]
                repinfo = data.info(objname)
                ci_levels = np.nan
                ci_val = np.nan
                if not opt.ignore_ci and 'confidence_intervals' in repinfo:
                    levels = repinfo['confidence_intervals']['levels']
                    ci_levels = (min(levels), max(levels)) if opt.confidence_interval is None else tuple(opt.confidence_interval)
                    ci1, ci2 = (val[1][levels.index(ci_levels[0])], val[1][levels.index(ci_levels[1])])
                    if isinstance(val[0], list):
                        val[0] = tuple(val[0])
                        ci1 = tuple(ci1)
                        ci2 = tuple(ci2)
                    ci_val = (ci1, ci2)
                frame['value'].append(val[0])
                frame['ci_levels'].append(ci_levels)
                frame['confidence_interval'].append(ci_val)


    xname = opt.names[opt.xvalue] if opt.xvalue in opt.names else opt.xvalue
    if opt.xlabel is None:
        opt.xlabel = xname

    if opt.xvalue != 'Time' and xname not in frame['vector_name']:
        for file in opt.filenames:
            data = mooseutils.ReporterReader(file)

            times = opt.times if len(opt.times) else data.times()
            for time in times:
                data.update(time)
                for var in data.variables():
                    if var[1] == opt.xvalue:
                        for stat in set(frame['statistic']):
                            frame['time'].append(time)
                            frame['object_name'].append(var[0])
                            frame['vector_name'].append(opt.xlabel)
                            frame['statistic'].append(stat)
                            frame['value'].append(tuple(data[var]))
                            frame['ci_levels'].append(np.nan)
                            frame['confidence_interval'].append(np.nan)
                        break
    elif xname in frame['vector_name'] and xname != opt.xlabel:
        for vn in frame['vector_name']:
            if vn == xname:
                vn = opt.xlabel


    frame = pd.DataFrame(frame)

    if opt.format == 0:
        textTable(frame, opt.number_format, opt.output)
    elif opt.format == 1:
        markdownTable(frame, opt.number_format, opt.output)
    elif opt.format == 2:
        latexTable(frame, opt.number_format, opt.output)
    elif opt.format == 3:
        barPlot(frame, opt.output)
    elif opt.format == 4:
        linePlot(frame, opt.xlabel, opt.output)

if __name__ == '__main__':
    sys.exit(main())
