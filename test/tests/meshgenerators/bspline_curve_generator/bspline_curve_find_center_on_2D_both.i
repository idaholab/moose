[Mesh]
  [gmg1]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 0
    xmin = -1
    ymax = 0.8
    ymin = 0
    nx = 2
    ny = 2
  []
  [gmg2]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 4.5
    xmin = 3.5
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
  []
  [bscg]
    type = BSplineCurveGenerator
    boundary_providing_start_point = 'top'
    start_mesh = gmg1
    boundary_providing_end_point = 'left'
    end_mesh = gmg2
    num_elements = 20
  []
[]
