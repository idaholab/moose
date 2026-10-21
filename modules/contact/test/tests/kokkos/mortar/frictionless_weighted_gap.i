# Frictionless mortar contact of a block bouncing on a long bottom block, posed on the current
# configuration with a Lagrange multiplier at the secondary nodes and Kokkos objects throughout.
# KokkosDiffusion of each displacement component stands in for the mechanics. The problem is that of
# bouncing-block-contact/frictionless-weighted-gap.i with correct edge dropping, and the gold file is
# a link to the gold file of that test.
starting_point = 2e-1
offset = 1e-2

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  file = ../../bouncing-block-contact/long-bottom-block-1elem-blocks-coarse.e
[]

[Variables]
  [disp_x]
    block = '1 2'
  []
  [disp_y]
    block = '1 2'
  []
  [normal_lm]
    block = 3
  []
[]

[ICs]
  [disp_y]
    block = 2
    variable = disp_y
    value = '${fparse starting_point + offset}'
    type = ConstantIC
  []
[]

[Kernels]
  [disp_x]
    type = KokkosDiffusion
    variable = disp_x
  []
  [disp_y]
    type = KokkosDiffusion
    variable = disp_y
  []
[]

[UserObjects]
  [weighted_gap_uo]
    type = KokkosWeightedGapUserObject
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    lm_variable = normal_lm
  []
[]

[Constraints]
  [weighted_gap_lm]
    type = KokkosComputeWeightedGapLMMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = normal_lm
    c = 1
    weighted_gap_uo = weighted_gap_uo
  []
  [normal]
    type = KokkosNormalMortarMechanicalContact
    primary_boundary = 20
    secondary_boundary = 10
    primary_subdomain = 4
    secondary_subdomain = 3
    variable = normal_lm
  []
[]

[Functions]
  [top]
    type = KokkosParsedFunction
    expression = '${starting_point} * cos(2 * pi / 40 * t) + ${offset}'
  []
  [left]
    type = KokkosParsedFunction
    expression = '1e-2 * t'
  []
[]

[BCs]
  [botx]
    type = KokkosDirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  []
  [boty]
    type = KokkosDirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
  []
  [topy]
    type = KokkosFunctionDirichletBC
    variable = disp_y
    boundary = 30
    function = top
    preset = false
  []
  [leftx]
    type = KokkosFunctionDirichletBC
    variable = disp_x
    boundary = 50
    function = left
    preset = false
  []
[]

[Executioner]
  type = Transient
  end_time = 50
  dt = 5
  dtmin = 5
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
  l_max_its = 30
  nl_max_its = 20
  line_search = 'none'
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
