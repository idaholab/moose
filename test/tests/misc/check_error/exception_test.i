[Mesh]
  file = square.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./exception]
    type = ExceptionKernel
    variable = u
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

[Preconditioning]
  active = ' '
  [./PBP]
    type = PBP
    solve_order = 'u'
    preconditioner  = 'LU'
    petsc_options = '-ksp_monitor'  # Test petsc options in PBP block
  []
[]

[Executioner]
  type = ExceptionSteady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]


