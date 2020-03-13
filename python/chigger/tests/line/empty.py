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

line = chigger.graphs.Line()
graph = chigger.graphs.Graph(line,
                            xaxis={'lim':[0,10], 'num_ticks':3, 'title':'x'},
                            yaxis={'lim':[0,30], 'num_ticks':5, 'title':'y'},
                            color_scheme='citrus', legend={'visible':False})

window = chigger.RenderWindow(graph, size=[500, 250], test=True)
window.write('empty.png')
window.start()
