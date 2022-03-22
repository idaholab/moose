[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Variables]
  [v]
    type = MooseVariableFVReal
    # breaks the constraint
    initial_condition = -1
  []
  [lambda]
    type = MooseVariableScalar
  []
[]

[FVKernels]
  [time]
    type = FVTimeKernel
    variable = v
  []
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
  [average]
    type = FVBoundedValueConstraint
    variable = v
    phi0 = 0
    lambda = lambda
    bound_type = 'HIGHER_THAN'
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
    value = 0
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
  type = Transient
  solve_type = 'Newton'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'

  num_steps = 2
  dt = 0.001
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
