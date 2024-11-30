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
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
    components = 2
  []
  [lambdab]
    order = CONSTANT
    family = MONOMIAL
    block = BOUNDARY_SIDE_LOWERD_SUBDOMAIN_EDGE2
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
    reaction_coefficient = re
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
    type = ArrayHFEMDiffusionTest
    variable = u
    lowerd_variable = lambda
    for_pjfnk = true
  []
[]

[BCs]
  [all]
    type = ArrayHFEMDirichletTestBC
    boundary = 'left right top bottom'
    variable = u
    lowerd_variable = lambdab
    for_pjfnk = true
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '1 1'
  []
  [re]
    type = GenericConstantArray
    prop_name = re
    prop_value = '0.1 0.1'
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
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    # we hide lambda because it may flip sign due to element
    # renumbering with distributed mesh
    type = Exodus
    hide = lambda
  []
[]
