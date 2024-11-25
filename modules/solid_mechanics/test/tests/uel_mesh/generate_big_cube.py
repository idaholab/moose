#!/usr/bin/env python

elem_nodes = [(-1.,          -1.),
              ( 1.,          -1.),
              (1.,           1.),
              (-1.,           1.),
              ( 0.,          -1.),
              (1.,           0.),
              (0.,           1.),
              (-1.,           0.),
              (0.,           0.)]

node_map = {}
node_id = 1
elem_id = 1

nodes = []
elems = []

N=10

# element patch
for i in range(N):
  for j in range(N):
    elem = [elem_id]
    elem_id += 1
    for nx, ny in elem_nodes:
      n = (nx + 2*i, ny + 2*j)
      if n in node_map:
        elem.append(node_map[n])
      else:
        node_map[n] = node_id
        nodes.append((node_id, n[0], n[1]))
        elem.append(node_id)
        node_id += 1
    elems.append(elem)

print("*Node")
for n in nodes:
  print(f'\t{n[0]},\t{n[1]},\t{n[2]}')

print("*Element, Type=U1, Elset=CUBE")
for e in elems:
  print(", ".join([str(v) for v in e]))
