[Mesh]
  [meshgen]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [v]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[FVKernels]
  [diff_u]
    type = FVDiffusion
    variable = u
    coeff = 1
  []
  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = 1
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [bottom]
    type = FVDirichletBC
    variable = v
    boundary = bottom
    value = 0
  []
  [top]
    type = FVDirichletBC
    variable = v
    boundary = top
    value = 1
  []
[]

[Problem]
  kernel_coverage_check = off
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[AuxVariables]
  [uv]
    order = CONSTANT
    family = MONOMIAL
    components = 2
  []
[]

[AuxKernels]
  [build_uv]
    type = BuildArrayVariableAux
    variable = uv
    component_variables = 'u v'
  []
[]

[Outputs]
  exodus = true
[]
