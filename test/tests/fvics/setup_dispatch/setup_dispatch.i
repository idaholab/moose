# FVInitialConditionWarehouse::initialSetup() had no caller, so initialSetup() was never
# dispatched to finite volume initial conditions. The initial condition here counts the call, and
# the postprocessor reports that count.

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 4
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
[]

[FVICs]
  [setup_ic]
    type = FVSetupTestIC
    variable = u
    value = 3
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = u
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
[]

[Postprocessors]
  [ic_initial]
    type = FVSetupCount
    object = setup_ic
    count_type = INITIAL
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
[]
