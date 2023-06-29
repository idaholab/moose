[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
[]

[Variables]
  [v]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
  []
[]

[AuxVariables]
  [epsilonr]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
  []

  [c]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
  []

  [z]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []

  [electrochem1]
    type = INSFVElectrochemicalPotential1
    variable = v
    epsilonr = epsilonr
    #coeff_interp_method = average
  []

  [electrochem2]
    type = INSFVElectrochemicalPotential2
    variable = v
    epsilonr = epsilonr
    c = 1
    z = 3
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 7
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 42
  []
[]

[Functions]
  [phi]
    type = ParsedFunction
    expression = x
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  residual_and_jacobian_together = true
[]

[Outputs]
  exodus = true
[]
