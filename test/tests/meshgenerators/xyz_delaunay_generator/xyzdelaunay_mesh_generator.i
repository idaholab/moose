[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
  []
  [outer_bdy]
    type = ParsedNodeTransformGenerator
    input = gmg
    x_function = "x"
    y_function = "y"
    z_function = "z+x*y*z"
  []
  [triang]
    type = XYZDelaunayGenerator
    boundary = 'outer_bdy'
    # Let NetGen know interior points are okay
    desired_volume = 100000
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
