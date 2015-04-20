[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 6
  xmin = -15.0
  xmax = 15.0
  ymin = -3.0
  ymax = 3.0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1
  [../]
[]

[Postprocessors]
  [./max_c]
    type = NodalMaxValue
    variable = c
    execute_on = 'initial timestep_end'
  [../]
[]

[UserObjects]
  [./arnold]
    type = Terminator
    expression = 'max_c < 0.5'
  [../]
[]

[Kernels]
  [./cres]
    type = Diffusion
    variable = c
  [../]

  [./time]
    type = TimeDerivative
    variable = c
  [../]
[]

[BCs]
  [./c]
    type = DirichletBC
    variable = c
    boundary = left
    value = 0
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  dt = 100
  num_steps = 6
[]

[Outputs]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]
