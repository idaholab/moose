[Mesh]
  file = 2-lines.e
  allow_renumbering = false
[]

[Variables]
  [u]
    family = LAGRANGE
    order = FIRST
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 3
  []
[]

[Constraints]
  [c1]
    type = LinearNodalConstraint
    variable = u
    primary = 0
    secondary_node_ids = 4
    penalty = 100000
    weights = 10
  []
[]

[Postprocessors]
  [u_secondary]
    type = NodalVariableValue
    variable = u
    nodeid = 4
  []

  [u_primary]
    type = NodalVariableValue
    variable = u
    nodeid = 0
  []

  [tie_error]
    type = ParsedPostprocessor
    expression = 'u_s - 10 * u_p'
    pp_names = 'u_secondary u_primary'
    pp_symbols = 'u_s u_p'
  []

  [num_nonlinear_iterations]
    type = NumNonlinearIterations
    outputs = none
  []

  [num_linear_iterations]
    type = NumLinearIterations
    outputs = none
  []

  [solve_time]
    type = PerfGraphData
    section_name = 'FEProblem::solve'
    data_type = TOTAL
    execute_on = 'TIMESTEP_END'
    outputs = none
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  csv = true
[]
