#!/usr/bin/env python3
# pylint: disable=missing-docstring
# This file is part of the MOOSE framework
# https://mooseframework.inl.gov
#
# All rights reserved, see COPYRIGHT for full restrictions
# https://github.com/idaholab/moose/blob/master/COPYRIGHT
#
# Licensed under LGPL 2.1, please see LICENSE for details
# https://www.gnu.org/licenses/lgpl-2.1.html

import chigger

# Create a graph
graph = chigger.graphs.Graph(
    xaxis={"lim": [0, 10], "num_ticks": 3, "title": "x"},
    yaxis={"lim": [0, 30], "num_ticks": 5, "title": "y"},
    color_scheme="citrus",
    legend={"visible": False},
)

# Generate data
x = range(0, 10)
y0 = range(0, 10)
y1 = range(0, 20, 2)
y2 = range(0, 30, 3)

# Create line objects
line0 = chigger.graphs.Line(x, y0, label="y0")
line1 = chigger.graphs.Line(x, y1, label="y1")
line2 = chigger.graphs.Line(x, y2, label="y2")

# Add lines to graph
graph.setOptions(lines=[line0, line1, line2])

# Window
window = chigger.RenderWindow(graph, size=[500, 250], test=True)
window.write("color.png")
window.start()
