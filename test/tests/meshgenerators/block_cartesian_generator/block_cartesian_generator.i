[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmin = 0
    xmax = 4
    ymin = 0
    ymax = 2.2
    nemesis = true
    output = true
  []
  [bcg]
    type = BlockCartesianGenerator
    input = 'gmg'
    dim = 2
    dx = '1.5 2.4 0.1'
    dy = '1.3 0.9'
    ix = '2 1 1'
    iy = '2 3'
    subdomain_id = '0 1 1 2 2 2'
  []
[]

[Outputs]
  
[]

