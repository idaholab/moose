#!/usr/bin/env python
import chigger

line = chigger.graphs.Line()
graph = chigger.graphs.Graph(line,
                            xaxis={'lim':[0,10], 'num_ticks':3, 'title':'x'},
                            yaxis={'lim':[0,30], 'num_ticks':5, 'title':'y'},
                            color_scheme='citrus', legend={'visible':False})

window = chigger.RenderWindow(graph, size=[500, 250], test=True)
window.write('empty.png')
window.start()
