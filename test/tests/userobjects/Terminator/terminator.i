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
    [./InitialCondition]
      type = RandomIC
      min = -1
      max =  1
    [../]
  [../]
[]

[Postprocessors]
  [./max_c]
    type = NodalMaxValue
    variable = c
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
  [./Periodic]
    [./c]
      variable = c
      auto_direction = 'x y'
    [../]
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  l_max_its = 30
  l_tol = 1.0e-3

  nl_max_its = 50
  nl_rel_tol = 1.0e-10

  dt = 0.1
  num_steps = 20
[]

[Outputs]
  file_base = out
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = false
  [../]
[]
