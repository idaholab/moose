#!/usr/bin/env python
import chigger

line = chigger.graphs.Line(x=[0,1], y=[2,4])
graph = chigger.graphs.Graph(line)
window = chigger.RenderWindow(graph, size=[300,300], test=True)
window.write('line.png')
window.start()
