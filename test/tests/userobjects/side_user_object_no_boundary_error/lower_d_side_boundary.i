[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 3
    ny = 3
    dim = 2
  []
  build_all_side_lowerd_mesh = true
[]

[Variables]
  [u]
    order = THIRD
    family = MONOMIAL
    block = 0
  []
  [uhat]
    order = CONSTANT
    family = MONOMIAL
    block = BOUNDARY_SIDE_LOWERD_SUBDOMAIN_EDGE2
  []
  [lambda]
    order = CONSTANT
    family = MONOMIAL
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
  []
  [lambdab]
    order = CONSTANT
    family = MONOMIAL
    block = BOUNDARY_SIDE_LOWERD_SUBDOMAIN_EDGE2
  []
[]

[AuxVariables]
  [v]
    order = CONSTANT
    family = MONOMIAL
    block = 0
    initial_condition = '1'
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = '1'
    block = 0
  []
  [source]
    type = CoupledForce
    variable = u
    v = v
    coef = '1'
    block = 0
  []
  [reaction]
    type = Reaction
    variable = uhat
    rate = '1'
    block = BOUNDARY_SIDE_LOWERD_SUBDOMAIN_EDGE2
  []
  [uhat_coupled]
    type = CoupledForce
    variable = uhat
    block = BOUNDARY_SIDE_LOWERD_SUBDOMAIN_EDGE2
    v = lambdab
    coef = '1'
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
    type = HFEMDirichletBC
    boundary = 'left right top bottom'
    variable = u
    lowerd_variable = lambdab
    uhat = uhat
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       basic                 mumps'
[]

[Postprocessors]
  [avg]
    type = SideAverageValue
    boundary = 'left right top bottom'
  []
[]
