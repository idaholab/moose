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
  []
  [v]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxVariables]
  [bounds_dummy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
  [reaction_u]
    type = Reaction
    variable = u
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
  [reaction_v]
    type = Reaction
    variable = v
  []
[]

[DGKernels]
  [dg_diff_u]
    type = ADDGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
    diff = 3
  []
  [dg_diff_v]
    type = ADDGDiffusion
    variable = v
    epsilon = -1
    sigma = 6
    diff = 4
  []
[]

[BCs]
  [left_u]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0'
    function = -0.5
    epsilon = -1
    sigma = 6
  []
  [right_u]
    type = NeumannBC
    variable = u
    boundary = 1
    value = 30
  []
  [left_v]
    type = DGFunctionDiffusionDirichletBC
    variable = v
    boundary = '0'
    function = 4
    epsilon = -1
    sigma = 6
  []
  [right_v]
    type = NeumannBC
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
  solve_type = 'PJFNK'
  petsc_options_iname = '-snes_type'
  petsc_options_value = 'vinewtonrsls'
[]

[Outputs]
  exodus = true
[]
