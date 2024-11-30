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
    block = 0
  []
  [reaction]
    type = Reaction
    variable = u
    rate = '1'
    block = 0
  []
  [source]
    type = BodyForce
    variable = u
    value = '1'
    block = 0
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

[Postprocessors]
  [intu]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = 0
  []
  [lambdanorm]
    type = ElementL2Norm
    variable = lambda
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
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
