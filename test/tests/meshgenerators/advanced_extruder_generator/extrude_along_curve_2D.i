[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 6
    xmax = 1
    xmin = -1
  []
  [bscg]
    type = BSplineCurveGenerator
    start_point = '0 0 0'
    end_point = '10 5 0'
    start_direction = '0 1 0'
    end_direction = '-1 0 0'
    num_elements = 20
  []
  [ext_along_curve]
    type = AdvancedExtruderGenerator
    input = gmg
    extrusion_curve = bscg
    end_extrusion_direction = '1 0 0'
    start_extrusion_direction = '0 1 0'
  []
[]
