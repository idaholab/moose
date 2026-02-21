[Mesh]
  [gmg1]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 6
    xmax = 1
    xmin = -1
  []
  [gmg2_base]
    type = GeneratedMeshGenerator
    dim = 1
    xmax = 2
    xmin = -2
    nx = 6
  []
  [gmg2_rotate]
    type = TransformGenerator
    input = gmg2_base
    transform = ROTATE
    vector_value = '90 0 0'
  []
  [gmg2]
    type = TransformGenerator
    input = gmg2_rotate
    transform = TRANSLATE
    vector_value = '10 5 0'
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
    input = gmg1
    extrusion_curve = bscg
    end_extrusion_direction = '1 0 0'
    start_extrusion_direction = '0 1 0'
    target_mesh = gmg2
  []
[]
