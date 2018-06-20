###########################################################
# This is a test that demonstrates a user-defined
# constraint. It forces variables in overlapping portion of
# two blocks to have the same value
#
# @Requirement F1.90
###########################################################


[Mesh]
  file = gold/2D_2D.e
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
    type = EqualValueEmbededConstraint
    slave = 2
    master = 1
    penalty = 1e3
    master_variable = phi
    variable = phi
    formulation = penalty
  []
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu       101'
  line_search = none
  nl_rel_tol = 1e-15
  nl_abs_tol = 1e-8
  l_max_its = 100
  nl_max_its = 10
[]

[Postprocessors]
  [nl_its]
    type = NumNonlinearIterations
  []
  [lin_its]
    type = NumLinearIterations
  []
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]
