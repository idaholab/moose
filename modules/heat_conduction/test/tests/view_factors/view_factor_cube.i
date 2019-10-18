[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  zmin = 0
  zmax = 1
  nx = 2
  ny = 2
  nz = 2
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./u]
  [../]
[]

[UserObjects]
  [./view_factor]
    type = UnobstructedPlanarViewFactor
    boundary = 'left right front back bottom top'
    execute_on = 'INITIAL'
  [../]
[]

[Postprocessors]
  [./left_right]
    type = ViewFactorPP
    from_boundary = left
    to_boundary = right
    view_factor_object_name = view_factor
  [../]

  [./left_top]
    type = ViewFactorPP
    from_boundary = left
    to_boundary = top
    view_factor_object_name = view_factor
  [../]

  [./left_back]
    type = ViewFactorPP
    from_boundary = left
    to_boundary = back
    view_factor_object_name = view_factor
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
