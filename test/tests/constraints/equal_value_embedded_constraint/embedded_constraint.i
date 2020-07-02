###########################################################
# This is a test that demonstrates a user-defined
# constraint. It forces variables in overlapping portion of
# two blocks to have the same value
###########################################################

[Mesh]
[]

[Variables]
  [phi]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = phi
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = phi
    boundary = 1
    value = 10.0
  []
  [bottom]
    type = DirichletBC
    variable = phi
    boundary = 2
    value = 0.0
  []
  [left]
    type = DirichletBC
    variable = phi
    boundary = 3
    value = 10.0
  []
  [right]
    type = DirichletBC
    variable = phi
    boundary = 4
    value = 0.0
  []
[]

[Constraints]
  [equal]
    type = EqualValueEmbeddedConstraint
    secondary = 2
    primary = 1
    penalty = 1e3
    primary_variable = phi
    variable = phi
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = none
  nl_rel_tol = 1e-15
  nl_abs_tol = 1e-8
  l_max_its = 100
  nl_max_its = 10
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
