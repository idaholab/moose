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
    family = LAGRANGE_VEC
  [../]
  [./v]
    block = '1'
    family = LAGRANGE_VEC
  [../]
  [w]
    family = LAGRANGE_VEC
  []
[]

[Kernels]
  [./diff_u]
    type = VectorDiffusion
    variable = u
    block = 0
  [../]
  [./diff_v]
    type = VectorDiffusion
    variable = v
    block = 1
  [../]
  [diff_w]
    type = VectorDiffusion
    variable = w
  []
[]

[InterfaceKernels]
  [./interface]
    type = ADVectorCoupledInterfacialSource
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
    type = VectorDirichletBC
    variable = u
    boundary = 'left'
    values = '0 0 0'
  [../]
  [./right]
    type = VectorDirichletBC
    variable = v
    boundary = 'right'
    values = '10 0 0'
  [../]
  [./middle]
    type = ADVectorMatchedValueBC
    variable = v
    boundary = 'primary0_interface'
    v = u
  [../]
  [w_left]
    type = VectorDirichletBC
    variable = w
    boundary = 'left'
    values = '0 0 0'
  []
  [w_right]
    type = VectorDirichletBC
    variable = w
    boundary = 'right'
    values = '4 0 0'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
