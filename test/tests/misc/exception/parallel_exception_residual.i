[Mesh]
  file = 2squares.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./exception]
    type = ExceptionKernel
    variable = u
    block = 1
    when = residual
  [../]
  [./diff]
    type = Diffusion
    variable = u
    block = 2
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
  [./right2]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
[]

[Executioner]
  type = TestSteady
  solve_type = 'PJFNK'
[]

[Outputs]
  output_initial = true
  exodus = true
  print_linear_residuals = true
  print_perf_log = true
[]
