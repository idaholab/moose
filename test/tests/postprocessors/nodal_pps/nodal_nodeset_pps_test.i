[Mesh]
  file = block_nodeset.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[DiracKernels]
  [./point_source_left]
    type = ConstantPointSource
    variable = u
    value = 1.0
    point = '0.2 0.2'
  [../]

  [./point_source_right]
    type = ConstantPointSource
    variable = u
    value = 2.0
    point = '0.8 0.8'
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 0
  [../]
[]

[Postprocessors]
  # This postprocessor will search all nodes in the mesh since it isn't restricted to a nodeset
  [./global_max_value]
    type = NodalMaxValue
    variable = u
  [../]

  # This postprocessor will only act on the specified nodeset so it will find a different max value
  [./left_max_value]
    type = NodalMaxValue
    variable = u
    boundary = 'left_side_nodes'
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
