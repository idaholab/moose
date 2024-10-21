[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = TET4
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
  [outer_bdy]
    type = ParsedNodeTransformGenerator
    input = gmg
    x_function = "z+x"
    y_function = "y"
    z_function = "z-x"
  []
  [hole_1]
    type = ParsedNodeTransformGenerator
    input = gmg
    x_function = ".25+.125*x"
    y_function = ".25+.125*y"
    z_function = ".25+.125*z"
  []
  [hole_1_name]
    type = RenameBlockGenerator
    input = hole_1
    old_block = 0
    new_block = hole
  []
  [hole_2]
    type = ParsedNodeTransformGenerator
    input = gmg
    x_function = ".75+.125*x"
    y_function = ".75+.125*y"
    z_function = ".75+.125*z"
  []
  [hole_2_name_1]
    type = RenameBlockGenerator
    input = hole_2
    old_block = 0
    new_block = 1
  []
  [hole_2_name_2]
    type = RenameBlockGenerator
    input = hole_2_name_1
    old_block = 1
    new_block = hole
  []
  [triang]
    type = XYZDelaunayGenerator
    boundary = 'outer_bdy'
    holes = 'hole_1_name
             hole_2'
    stitch_holes = 'true
                    false'
    desired_volume = 10000
    output_subdomain_name = "triangles"
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
