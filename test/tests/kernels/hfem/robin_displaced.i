[Mesh]
  [square]
    type = CartesianMeshGenerator
    dx = '0.3125 0.3125 0.3125'
    dy = '0.3333333333333 0.3333333333333 0.3333333333333'
    dim = 2
  []
  displacements = 'x_disp y_disp'
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
[]

[AuxVariables]
  [v]
    order = CONSTANT
    family = MONOMIAL
    block = 0
    initial_condition = '1'
  []
  [x_disp]
    block = 0
  []
  [y_disp]
    block = 0
    initial_condition = 0
  []
[]

[AuxKernels]
  [x_disp]
    type = ParsedAux
    variable = x_disp
    use_xyzt = true
    expression = 'x/15'
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    block = 0
    diffusion_coefficient = dc
    use_displaced_mesh = true
  []
  [source]
    type = ArrayCoupledForce
    variable = u
    v = v
    coef = '1 2'
    block = 0
    use_displaced_mesh = true
  []
[]

[DGKernels]
  [surface]
    type = ArrayHFEMDiffusion
    variable = u
    lowerd_variable = lambda
    use_displaced_mesh = true
  []
[]

[BCs]
  [all]
    type = ArrayVacuumBC
    boundary = 'left right top bottom'
    variable = u
    use_displaced_mesh = true
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
    block = 0
    use_displaced_mesh = true
  []
  [lambdanorm]
    type = ElementArrayL2Norm
    variable = lambda
    block = INTERNAL_SIDE_LOWERD_SUBDOMAIN_EDGE2
    use_displaced_mesh = true
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
