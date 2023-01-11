[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 4
  ny = 4
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  [../]
[]

[Functions]
  [./force_fn]
    type = ParsedFunction
    expression = '1'
  [../]

  [./exact_fn]
    type = ParsedFunction
    expression = 't'
  [../]
[]

[Kernels]
  [./time_u]
    type = TimeDerivative
    variable = u
  [../]

  [./diff_u]
    type = Diffusion
    variable = u
  [../]

  [./ffn_u]
    type = BodyForce
    variable = u
    function = force_fn
  [../]
[]

[BCs]
  [./all_u]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]

[Postprocessors]
  [./a]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'initial timestep_end'
  [../]

  [./total_a]
    type = TimeIntegratedPostprocessor
    value = a
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  dt = 1
  start_time = 1
  end_time = 3
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
