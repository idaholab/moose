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

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = fv_out
  exodus = true
[]
