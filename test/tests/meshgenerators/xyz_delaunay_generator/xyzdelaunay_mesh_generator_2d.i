[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'gold/2d_cube.e'
  []
  [outer_bdy]
    type = ParsedNodeTransformGenerator
    input = fmg
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
  [output]
    type = CSV
    file_base = 'xyzdelaunay_mesh_generator_out'
  []
[]
