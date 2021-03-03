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
    initial_condition = 1
  []
  [v]
    block = '1'
    initial_condition = 0
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
    block = 0
  []
  [diff_v]
    type = Diffusion
    variable = v
    block = 1
  []
[]

[InterfaceKernels]
  [interface]
    type = ADMaterialPropertySource
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    source = couple
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  []
  [right]
    type = DirichletBC
    variable = v
    boundary = 'right'
    value = 0
  []
[]

[Materials]
  [consumer]
    type = ConsumerInterfaceMaterial
    prop_consumed = ad_jump
    prop_produced = couple
    boundary = primary0_interface
  []
  [jump]
    type = JumpInterfaceMaterial
    var = u
    neighbor_var = v
    boundary = primary0_interface
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
