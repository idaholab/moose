[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 4
[]

[Variables]
  [v]
    type = MooseVariableFVReal
  []
  [lambda]
    type = MooseVariableScalar
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
  [average]
    type = FVIntegralValueConstraint
    variable = v
    phi0 = phi0_pp
    lambda = lambda
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 7
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
[]

[Postprocessors]
  [phi0_pp]
    type = FunctionValuePostprocessor
    function = 't + 13'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 2
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
[]

[Outputs]
  exodus = true
[]
