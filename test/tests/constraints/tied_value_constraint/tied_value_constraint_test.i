# [Debug]
# show_top_residuals = 5
# []

[Mesh]
  type = FileMesh
  file = constraint_test.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  # active = 'diff'
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  # active = 'left right'
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  [../]
[]

[Constraints]
  [./value]
    type = TiedValueConstraint
    variable = u
    secondary = 2
    primary = 3
    primary_variable = u
  [../]
[]

[Preconditioning]
  # active = 'FDP'
  active = ''
  [./FDP]
    # full = true
    # off_diag_row    = 'v'
    # off_diag_column = 'u'
    type = FDP
  [../]
[]

[Executioner]
  # l_tol = 1e-1
  # l_tol = 1e-
  # nl_rel_tol = 1e-14
  type = Steady
  solve_type = NEWTON
  l_max_its = 100
[]

[Outputs]
  file_base = out
  exodus = true
[]

