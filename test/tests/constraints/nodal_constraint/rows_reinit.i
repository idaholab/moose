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
    formulation = rows
    weights = 1
  []
[]

[Controls]
  [ramp_weights]
    type = RealVectorFunctionControl
    vector_type = 'std::vector<Real>'
    function = 't'
    parameter = 'Constraints/c1/weights'
    execute_on = 'initial timestep_begin'
  []
[]

[Postprocessors]
  [u_secondary]
    type = NodalVariableValue
    variable = u
    nodeid = 4
    execute_on = 'timestep_end'
  []

  [u_primary]
    type = NodalVariableValue
    variable = u
    nodeid = 0
    execute_on = 'timestep_end'
  []

  [ratio]
    type = ParsedPostprocessor
    expression = 'u_s / u_p'
    pp_names = 'u_secondary u_primary'
    pp_symbols = 'u_s u_p'
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2

  solve_type = 'PJFNK'
[]

[Outputs]
  csv = true
  execute_on = 'timestep_end'
[]
