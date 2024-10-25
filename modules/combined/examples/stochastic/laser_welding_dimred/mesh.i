[Mesh]
  [cmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = ${xmin}
    xmax = ${xmax}
    ymin = ${fparse ymin}
    ymax = 0
    nx = 161
    ny = 50
  []
  displacements = 'disp_x disp_y'
[]
