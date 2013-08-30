[Mesh]
  file = square.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = PPSDiffusion
    variable = u
    pps_name = 'nonexistent_pps'
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Postprocessors]
[]

[Executioner]
  type = Steady

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'


[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]


