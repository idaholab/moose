#!/usr/bin/env python
import chigger

line = chigger.graphs.Line(x=[1], y=[2], marker='circle', label='y-data')
graph = chigger.graphs.Graph(line)
graph.setOptions('xaxis', lim=[0,3])
graph.setOptions('yaxis', lim=[0,3])

window = chigger.RenderWindow(graph, size=[300,300], test=True)
window.write('single_point.png')
window.start()
