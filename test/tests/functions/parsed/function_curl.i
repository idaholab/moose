[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = -1
  ymin = -1
  elem_type = QUAD9
[]

[Variables]
  # u = (y, -x, 0)
  [./u]
    family = NEDELEC_ONE
    order = FIRST
  [../]
[]

[Functions]
  # Simple "clockwise rotating" field in XY plane. curl(u) = (0, 0, -2)
  [./field]
    type = ParsedVectorFunction
    expression_x = 'y'
    expression_y = '-x'
    curl_z = '-2'
  [../]
  [./ffn_x]
    type = ParsedFunction
    expression = 'y'
  [../]
  [./ffn_y]
    type = ParsedFunction
    expression = '-x'
  [../]
[]

[Kernels]
  [./diff]
    type = VectorFEWave
    variable = u
    x_forcing_func = ffn_x
    y_forcing_func = ffn_y
  [../]
[]

[BCs]
  [./top]
    type = VectorCurlBC
    curl_value = field
    variable = u
    boundary = 'left right top bottom'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
