[Mesh]
  [inner_cube]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = TET4
    nx = 1
    ny = 1
    nz = 1
    xmin = -0.4
    xmax = 0.4
    ymin = -0.4
    ymax = 0.4
    zmin = -0.4
    zmax = 0.4
  []
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    zmin = -1
    zmax = 1
  []
  [layer_2_bdy]
    type = ParsedNodeTransformGenerator
    input = gmg
    x_function = "z+x"
    y_function = "y"
    z_function = "z-x"
  []
  [layer_3_bdy]
    type = ParsedNodeTransformGenerator
    input = gmg
    x_function = "3*x"
    y_function = "3*y"
    z_function = "3*z"
  []
  [layer_4_bdy]
    type = ParsedNodeTransformGenerator
    input = gmg
    x_function = "4*x"
    y_function = "4*y"
    z_function = "4*z"
  []
  [triang_2]
    type = XYZDelaunayGenerator
    boundary = 'layer_2_bdy'
    holes = 'inner_cube'
    stitch_holes = 'true'
    desired_volume = 0.1
  []
  [triang_3]
    type = XYZDelaunayGenerator
    boundary = 'layer_3_bdy'
    holes = 'triang_2'
    stitch_holes = 'true'
    desired_volume = 0.2
  []
  [triang_4]
    type = XYZDelaunayGenerator
    boundary = 'layer_4_bdy'
    holes = 'triang_3'
    stitch_holes = 'true'
    desired_volume = 0.4
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [volume]
    type = VolumePostprocessor
  []
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
[]
