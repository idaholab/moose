[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  nz = 6
[]

[Variables]
  [./u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  [../]
[]

[AuxVariables]
  [./layered_integral]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[FVKernels]
  [./diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  [../]
[]

[FVBCs]
  [./bottom]
    type = FVDirichletBC
    variable = u
    boundary = bottom
    value = 0
  [../]
  [./top]
    type = FVDirichletBC
    variable = u
    boundary = top
    value = 1
  [../]
[]

[AuxKernels]
  [./liaux]
    type = SpatialUserObjectAux
    variable = layered_integral
    boundary = right
    user_object = layered_integral
  [../]
[]

[UserObjects]
  [./layered_integral]
    type = LayeredSideIntegral
    direction = y
    num_layers = 3
    variable = u
    execute_on = linear
    boundary = right
  [../]
[]

[Executioner]
  type = Steady
  nl_abs_tol = 1e-14
  nl_rel_tol = 1e-14
  l_abs_tol = 1e-14
  l_tol = 1e-6
[]

[Outputs]
  exodus = true
[]
