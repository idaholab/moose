[Problem]
  previous_nl_solution_required = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  []
  [v]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  []
[]

[Kernels]
  [td_u]
    type = TimeDerivative
    variable = u
  []
  [source_u]
    type = Reaction
    variable = u
    rate = 0.1
  []

  [td_v]
    type = TimeDerivative
    variable = v
  []
  [source_v]
    type = CoupledForceLagged
    variable = v
    v = u
    coefficient = -0.1
  []
[]

[Postprocessors]
  [u]
    type = ElementAverageValue
    variable = u
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [v]
    type = ElementAverageValue
    variable = v
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 1
  solve_type = 'NEWTON'
[]

[Outputs]
  csv = true
[]
