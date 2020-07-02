[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
    xmax = 2
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  [../]
  [./interface]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  [../]
[]

[Variables]
  [./u]
    block = '0'
  [../]
  [./v]
    block = '1'
  [../]
  [w][]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
    block = 0
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
    block = 1
  [../]
  [diff_w]
    type = Diffusion
    variable = w
  []
[]

[InterfaceKernels]
  [./interface]
    type = ADCoupledInterfacialSource
    variable = u
    neighbor_var = v
    var_source = w
    boundary = primary0_interface
    D = 1
    D_neighbor = 1
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = v
    boundary = 'right'
    value = 10
  [../]
  [./middle]
    type = MatchedValueBC
    variable = v
    boundary = 'primary0_interface'
    v = u
  [../]
  [w_left]
    type = DirichletBC
    variable = w
    boundary = 'left'
    value = 0
  []
  [w_right]
    type = DirichletBC
    variable = w
    boundary = 'right'
    value = 4
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
