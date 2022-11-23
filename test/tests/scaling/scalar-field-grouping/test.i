[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
  [interface_primary_side]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary_interface'
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    block = 0
    initial_condition = 0.5
  []
  [v]
    type = MooseVariableFVReal
    block = 1
    initial_condition = 0.5
  []
  [lambda]
    type = MooseVariableScalar
  []
[]

[FVKernels]
  [diff_left]
    type = FVDiffusion
    variable = u
    coeff = 'left'
    block = 0
  []
  [diff_right]
    type = FVDiffusion
    variable = v
    coeff = 'right'
    block = 1
  []
[]

[FVInterfaceKernels]
  [interface]
    type = FVTwoVarContinuityConstraint
    variable1 = u
    variable2 = v
    boundary = 'primary_interface'
    subdomain1 = '0'
    subdomain2 = '1'
    lambda = 'lambda'
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [v_left]
    type = FVDirichletBC
    variable = v
    boundary = 'right'
    value = 0
  []
[]

[Materials]
  [block0]
    type = ADGenericFunctorMaterial
    block = '0'
    prop_names = 'left'
    prop_values = '1'
  []
  [block1]
    type = ADGenericFunctorMaterial
    block = '1'
    prop_names = 'right'
    prop_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type'
  petsc_options_value = 'asm lu NONZERO'
  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
  scaling_group_variables = 'u v lambda'
  verbose = true
[]

[Outputs]
  exodus = true
[]
