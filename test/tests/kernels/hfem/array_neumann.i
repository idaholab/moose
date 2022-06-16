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
    components = 2
  []
  [lambda]
    order = CONSTANT
    family = MONOMIAL
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN
    components = 2
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
    type = ArrayDiffusion
    variable = u
    block = 0
    diffusion_coefficient = dc
  []
  [reaction]
    type = ArrayReaction
    variable = u
    block = 0
    reaction_coefficient = rc
  []
  [source]
    type = ArrayCoupledForce
    variable = u
    v = v
    coef = '1 2'
    block = 0
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
    type = ArrayNeumannBC
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
  [rc]
    type = GenericConstantArray
    prop_name = rc
    prop_value = '1 1'
  []
[]

[Postprocessors]
  [intu]
    type = ElementIntegralArrayVariablePostprocessor
    variable = u
    block = 0
  []
  [lambdanorm]
    type = ElementArrayL2Norm
    variable = lambda
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -snes_linesearch_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu       basic                 mumps'
[]

[Outputs]
  [out]
    # we hide lambda because it may flip sign due to element
    # renumbering with distributed mesh
    type = Exodus
    hide = lambda
  []
[]
