#!/usr/bin/env python
import chigger

x = [0, 1, 2, 3, 4, 5]
y = [0, 1, 4, 9, 16, 25]

line = chigger.graphs.Line(label='x^2', color=[0,1,0], tracer=True)
graph = chigger.graphs.Graph(line)
graph.setOptions('xaxis', lim=[0,6])
graph.setOptions('yaxis', lim=[0,28])
graph.setOptions('legend', visible=True)
window = chigger.RenderWindow(graph, size=[400,400], test=True)

for i in range(len(x)):
    line.setOptions(x=[x[i]], y=[y[i]])
    window.write('tracer_' + str(i) + '.png')
window.start()
