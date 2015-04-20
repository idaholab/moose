[Mesh]
  type = GeneratedMesh
  nx = 10
  ny = 10
  dim = 2
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

[ICs]
  [./ds_ic]
    type = DataStructIC
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  nl_rel_tol = 1e-12
[]

[Outputs]
  output_initial = true
  exodus = true
  print_perf_log = true
[]
