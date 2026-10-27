[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 1
    subdomain_ids = '1 2'
  []
[]

[AuxVariables]
  [source_value]
    family = MONOMIAL
    order = CONSTANT
  []

  [source_gradient]
    family = LAGRANGE
    order = FIRST
  []
[]

[AuxKernels]
  [set_source_value]
    type = ParsedAux
    variable = source_value
    expression = 'if(x < 0.5, 3 + 2*t, 5 + 10*t)'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []

  [set_source_gradient]
    type = ParsedAux
    variable = source_gradient
    expression = 'if(x < 0.5, (1 + t)*x, 0.5*(1 + t) + (2 + 3*t)*(x - 0.5))'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[Executioner]
  type = Transient
  start_time = 0
  num_steps = 1
  dt = 1
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]
