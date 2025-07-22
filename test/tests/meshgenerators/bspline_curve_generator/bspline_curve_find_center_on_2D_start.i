[Mesh]
  [gmg1]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    xmin = 0
    ymax = 1
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
    start_boundary = 'top'
    start_mesh = gmg1
    end_point = '4 1 0'
    start_direction = '0 1 0'
    end_direction = '-2 1 0'
    num_elements = 10
  []
  [connector]
    type = CombinerGenerator
    inputs = 'gmg1 bscg gmg2'
  []
[]