###########################################################
# This is a test that demonstrates a user-defined
# constraint. It forces variables in overlapping portion of
# two blocks to have the same value
###########################################################

[Mesh]
  file=gold/2D_2D.e
[]

[Variables]
  [phi]
    order = FIRST
    family = LAGRANGE
  []
  [phi1]
    order = FIRST
    family = LAGRANGE
    block = 1
  []
  [phi2]
    order = FIRST
    family = LAGRANGE
    block = 2
    # scaling = 10000
  []
[]

[Kernels]
  [diffusion]
    type = Diffusion
    variable = phi
  []
  [diffusion1]
    type = Diffusion
    variable = phi1
    block = 1
  []
  [diffusion2]
    type = Diffusion
    variable = phi2
    block = 2
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
  [top_block1]
    type = DirichletBC
    variable = phi1
    boundary = 1
    value = 10.0
  []
  [bottom_block1]
    type = DirichletBC
    variable = phi1
    boundary = 2
    value = 0.0
  []
  [left_block2]
    type = DirichletBC
    variable = phi2
    boundary = 3
    value = 10.0
  []
  [right_block2]
    type = DirichletBC
    variable = phi2
    boundary = 4
    value = 0.0
  []
[]

[Constraints]
  [equal]
    type = ADEqualValueEmbeddedConstraint
    secondary = 2
    primary = 1
    penalty = 1e3
    primary_variable = phi
    variable = phi
    formulation = penalty
  []
  [equal_phi1_phi2]
    type = ADEqualValueEmbeddedConstraint
    secondary = 2
    primary = 1
    penalty = 1e3
    primary_variable = phi1
    variable = phi2
    formulation = penalty
  []
[]

[Postprocessors]
  [l2_difference_block_1]
    type = ElementL2Difference
    variable = phi
    other_variable = phi1
    block = 1
  []
  [l2_difference_block_2]
    type = ElementL2Difference
    variable = phi
    other_variable = phi2
    block = 2
  []
  [compare]
    type = ParsedPostprocessor
    expression = 'abs(l2_difference_block_1)+abs(l2_difference_block_2)'
    pp_names = 'l2_difference_block_1 l2_difference_block_2'
  []
[]

[UserObjects]
  [terminate]
    type = Terminator
    expression = 'compare > 1e-10'
    error_level = ERROR
    message = 'Two variable solution does match single variable solution to requried tolerance.'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = none
  nl_rel_tol = 1e-15
  nl_abs_tol = 1e-8
  l_max_its = 100
  nl_max_its = 10
[]

[Outputs]
  print_linear_residuals = false
[]
