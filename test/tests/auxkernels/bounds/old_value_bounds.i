[Mesh]
  type = GeneratedMesh

  dim = 2

  xmin = 0
  xmax = 1

  ymin = 0
  ymax = 1

  nx = 10
  ny = 10
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []

  [v]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [bounds_dummy]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [time_u]
    type = TimeDerivative
    variable = u
  []
  [diff_u]
    type = Diffusion
    variable = u
  []

  [time_v]
    type = TimeDerivative
    variable = v
  []
  [diff_v]
    type = Diffusion
    variable = v
  []
[]

[BCs]
  [left_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  []

  [right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []

  [left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 0
  []

  [right_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 1
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
    type = VariableOldValueBoundsAux
    variable = bounds_dummy
    bounded_variable = u
    bound_type = lower
  []

  [v_upper_bound]
    type = ConstantBoundsAux
    variable = bounds_dummy
    bounded_variable = v
    bound_type = upper
    bound_value = 3
  []
  [v_lower_bound]
    type = VariableOldValueBoundsAux
    variable = bounds_dummy
    bounded_variable = v
    bound_type = lower
  []
[]

[Executioner]
  type = Transient
  num_steps = 2

  solve_type = 'PJFNK'
  petsc_options_iname = '-snes_type'
  petsc_options_value = 'vinewtonrsls'
[]

[Outputs]
  exodus = true
[]
