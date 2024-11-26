[Mesh]

  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = 0
    ymax = 2
    nx = 2
    ny = 2
    subdomain_ids = '1 2 3 4'
  []
  [1to2]
    type = SideSetsBetweenSubdomainsGenerator
    input = gmg
    primary_block = '1'
    paired_block = '2'
    new_boundary = 'onetwo'
  []
  [2to1]
    type = SideSetsBetweenSubdomainsGenerator
    input = 1to2
    primary_block = '2'
    paired_block = '1'
    new_boundary = 'twoone'
  []
  [1to3]
    type = SideSetsBetweenSubdomainsGenerator
    input = 2to1
    primary_block = '1'
    paired_block = '3'
    new_boundary = 'onethree'
  []
  [3to1]
    type = SideSetsBetweenSubdomainsGenerator
    input = 1to3
    primary_block = '3'
    paired_block = '1'
    new_boundary = 'threeone'
  []
  build_all_side_lowerd_mesh = true
[]

[Variables]
  [u]
    order = THIRD
    family = MONOMIAL
    block = '1 2 3 4'
  []
  [lambda]
    order = CONSTANT
    family = MONOMIAL
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = '1'
    block = '1 2 3 4'
  []
  [reaction]
    type = Reaction
    variable = u
    rate = '1'
    block = '1 2 3 4'
  []
  [source]
    type = BodyForce
    variable = u
    value = '1'
    block = '1 2 3 4'
  []
  [time]
    type = CoefTimeDerivative
    variable = u
    block = '1 2 3 4'
    Coefficient = 1
  []
[]

[DGKernels]
  [surface]
    type = HFEMDiffusion
    variable = u
    lowerd_variable = lambda
  []
[]

[BCs]
  [all]
    type = NeumannBC
    boundary = 'left right top bottom'
    variable = u
  []
[]

[UserObjects]
  [onetwo_uo]
    type = LowerDIntegralSideUserObject
    boundary = onetwo
    lowerd_variable = lambda

  []
  [twoone_uo]
    type = LowerDIntegralSideUserObject
    boundary = twoone
    lowerd_variable = lambda

  []
  [onethree_uo]
    type = LowerDIntegralSideUserObject
    boundary = onethree
    lowerd_variable = lambda

  []
  [threeone_uo]
    type = LowerDIntegralSideUserObject
    boundary = threeone
    lowerd_variable = lambda

  []
[]

[Postprocessors]
  [unorm]
    type = ElementL2Norm
    variable = u
    block = '1 2 3 4'
  []
[]

[Executioner]
  type = Transient
  nl_abs_tol = 1e-12
  num_steps = 8
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       basic                 mumps'
[]
