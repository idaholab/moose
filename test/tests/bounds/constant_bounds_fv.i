[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 1
  nx = 10
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
  [v]
    type = MooseVariableFVReal
  []
[]

[AuxVariables]
  [bounds_dummy]
    order = CONSTANT
    family = MONOMIAL
    fv = true
  []
[]

[FVKernels]
  [diff_u]
    type = FVDiffusion
    variable = u
    coeff = 4
  []
  [reaction_u]
    type = FVReaction
    variable = u
  []
  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = 2
  []
  [reaction_v]
    type = FVReaction
    variable = v
  []
[]

[FVBCs]
  [left_u]
    type = FVDirichletBC
    variable = u
    boundary = '0'
    value = -0.5
  []
  [right_u]
    type = FVNeumannBC
    variable = u
    boundary = 1
    value = 30
  []
  [left_v]
    type = FVDirichletBC
    variable = v
    boundary = '0'
    value = 4
  []
  [right_v]
    type = FVNeumannBC
    variable = v
    boundary = 1
    value = -40
  []
[]

[Bounds]
  [u_upper_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = u
    bound_type = upper
    bound_value = 1
  []
  [u_lower_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = u
    bound_type = lower
    bound_value = 0
  []
  [v_upper_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = v
    bound_type = upper
    bound_value = 3
  []
  [v_lower_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = v
    bound_type = lower
    bound_value = -1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-snes_type'
  petsc_options_value = 'vinewtonrsls'
[]

[Outputs]
  exodus = true
[]
