[Mesh]
  [gmg1]
    type = GeneratedMeshGenerator
    dim = 2
  []
  [bscg]
    type = BSplineCurveGenerator
    start_boundary = 'top'
    start_mesh=gmg1

    end_point = '4 0 0'
    start_direction = '0 1 0'
    end_direction = '-2 1 0'
    num_elements = 10
  []
  [connector]
    type = CombinerGenerator
    inputs = 'gmg1 bscg'
  []
[]