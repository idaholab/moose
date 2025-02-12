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
import shutil
import mooseutils

import plotly.graph_objects as go

def run():
    exec = shutil.which("moose-opt") or mooseutils.find_moose_executable_recursive()
    if not exec:
        raise AssertionError("Cannot find a valid MOOSE executable.")
    mooseutils.run_executable(exec, ["-i", "step9.i"])

def plot():
    pp_data = mooseutils.PostprocessorReader('step9_out.csv')
    times = pp_data["time"]
    time_hrs = times / 3600

    fig = go.Figure(
        data=go.Scatter(
            x=time_hrs,
            y=pp_data["max_temperature_concrete"] - 273,
        )
    )
    fig.update_layout(
        xaxis=dict(title=dict(text="Time (hours)")),
        yaxis=dict(title=dict(text="Concrete Max Temperature (C)")),
    )
    fig.show()

    fig = go.Figure(
        data=go.Scatter(
            x=time_hrs,
            y=pp_data["water_heat_flux"] / 1000,
        )
    )
    fig.update_layout(
        xaxis=dict(title=dict(text="Time (hours)")),
        yaxis=dict(title=dict(text="Water heat flux (kW/m2)")),
    )
    fig.show()

    vpp_data_x = mooseutils.VectorPostprocessorReader('step9_out_temperature_sample_x_*.csv')
    vpp_data_y = mooseutils.VectorPostprocessorReader('step9_out_temperature_sample_y_*.csv')

    figx = go.Figure()
    figy = go.Figure()
    for it in vpp_data_x.times()[::2]:
        vpp_data_x.update(it)
        vpp_data_y.update(it)

        label = f"Time = {int(time_hrs[it] / 24)} days"
        figx.add_trace(go.Scatter(x=vpp_data_x["x"], y=vpp_data_x["T"] - 273, name=label))
        figy.add_trace(go.Scatter(x=vpp_data_y["y"], y=vpp_data_y["T"] - 273, name=label))

    figx.update_layout(
        xaxis=dict(title=dict(text="x (m)")),
        yaxis=dict(title=dict(text="Temperature (C)")),
    )
    figy.update_layout(
        xaxis=dict(title=dict(text="y (m)")),
        yaxis=dict(title=dict(text="Temperature (C)")),
    )

    figx.show()
    figy.show()


if __name__ == '__main__':
    if not os.path.exists('step9_out.csv'):
        run()
    plot()
