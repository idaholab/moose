#!/usr/bin/env python3
#pylint: disable=missing-docstring
#* This file is part of the MOOSE framework
#* https://www.mooseframework.org
#*
#* All rights reserved, see COPYRIGHT for full restrictions
#* https://github.com/idaholab/moose/blob/master/COPYRIGHT
#*
#* Licensed under LGPL 2.1, please see LICENSE for details
#* https://www.gnu.org/licenses/lgpl-2.1.html

import chigger

cyl0 = chigger.geometric.CylinderSource(position=[0,0,0], height=3, radius=0.1, orientation=[0,0,0], color=[0.5,0,0])
cyl1 = chigger.geometric.CylinderSource(position=[0,0,0], height=2, radius=0.2, orientation=[90,90,0], color=[0,0.5,0], edges=True, edge_color=[1,1,1])
cyl2 = chigger.geometric.CylinderSource(position=[0,0,0], height=1, radius=0.3, orientation=[0,90,90], color=[0,0,0.5], edges=True, edge_color=[1,1,1])

cyls = chigger.base.ChiggerResult(cyl0, cyl1, cyl2)

window = chigger.RenderWindow(cyls, size=[300,300], test=True)
window.write('cylinder_source.png')
window.start()
