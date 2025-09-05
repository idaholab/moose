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

cyl0 = chigger.geometric.LineSource(
    point1=[0, 0, 0], point2=[0, 1, 0], data=[1, 2, 4, 8, 16], cmap="viridis"
)
cyls = chigger.base.ChiggerResult(cyl0)
window = chigger.RenderWindow(cyls, size=[300, 300], test=True)
window.write("line_source_data.png")
window.start()
