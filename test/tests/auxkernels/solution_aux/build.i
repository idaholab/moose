[Mesh]
  type = FileMesh
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
    type = Diffusion
    variable = u
  [../]
[]


[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 2
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 3
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-10
[]

[Outputs]
  output_initial = true
  exodus = true
  xda = true
  print_perf_log = true
[]
