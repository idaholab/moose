[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
    xmax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  []
  [interface]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  []
[]

[Variables]
  [u]
    block = '0'
  []
  [v]
    block = '1'
  []
[]

[Kernels]
  [diff_u]
    type = ADMatDiffusion
    variable = u
    diffusivity = 4
    block = 0
  []
  [diff_v]
    type = ADMatDiffusion
    variable = v
    diffusivity = 2
    block = 1
  []
[]

[InterfaceKernels]
  [penalty_interface]
    type = ADPenaltyInterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    penalty = 1e6
    jump_prop_name = "jump"
  []
[]

[Materials]
  [bulk]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'functor_var_mat_prop'
    subdomain_to_prop_value = '0 u 1 v'
  []
  [bulk_traditional]
    type = ScalarPropFromFunctorProp
    functor = 'functor_var_mat_prop'
    prop = 'var_mat_prop'
  []
  [jump]
    type = PropertyJumpInterfaceMaterial
    property = var_mat_prop
    boundary = primary0_interface
  []
[]

[BCs]
  [left]
    type = ADDirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [right]
    type = ADDirichletBC
    variable = v
    boundary = 'right'
    value = 0
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
