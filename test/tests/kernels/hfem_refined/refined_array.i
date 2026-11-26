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
    components = 2
  []
  [lambda]
    order = FIRST
    family = MONOMIAL
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
    components = 2
  []
[]

[AuxVariables]
  [v]
    order = CONSTANT
    family = MONOMIAL
    block = '0 1'
    initial_condition = 1
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
    block = '0 1'
  []
  [source]
    type = ArrayCoupledForce
    variable = u
    v = v
    coef = '1 2'
    block = '0 1'
  []
[]

[DGKernels]
  [surface]
    type = ArrayHFEMDiffusion
    variable = u
    lowerd_variable = lambda
  []
[]

[BCs]
  [all]
    type = ArrayVacuumBC
    boundary = 'left right top bottom'
    variable = u
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
[]

[Postprocessors]
  [intu]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    block = '0 1'
  []
  [lambdanorm]
    type = ElementArrayL2Norm
    variable = lambda
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
  []
  [intu_1]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    component = 1
    block = '0 1'
  []
  [lambdanorm_1]
    type = ElementArrayL2Norm
    variable = lambda
    component = 1
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
