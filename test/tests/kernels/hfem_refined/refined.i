[Problem]
  use_hash_table_matrix_assembly = true
[]

[Mesh]
  [square]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    subdomain_ids = '0 1 0 0'
  []
  [refine]
    type = RefineBlockGenerator
    input = square
    block = 1
    refinement = 1
  []
  build_all_side_lowerd_mesh = true
[]

[Variables]
  [u]
    order = THIRD
    family = MONOMIAL
    block = '0 1'
  []
  [lambda]
    order = FIRST
    family = MONOMIAL
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = 1
    block = '0 1'
  []
  [source]
    type = BodyForce
    variable = u
    value = 1
    block = '0 1'
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
    type = VacuumBC
    boundary = 'left right top bottom'
    variable = u
  []
[]

[Postprocessors]
  [intu]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = '0 1'
  []
  [lambdanorm]
    type = ElementL2Norm
    variable = lambda
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu mumps'
[]

[Outputs]
  csv = true
[]
