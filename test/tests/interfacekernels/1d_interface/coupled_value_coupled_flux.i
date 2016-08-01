[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  xmax = 2
[]

[MeshModifiers]
  [./subdomain1]
    type = SubdomainBoundingBox
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 1.0 0'
  [../]
  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = subdomain1
    master_block = '0'
    paired_block = '1'
    new_boundary = 'master0_interface'
  [../]
  [./interface_again]
    type = SideSetsBetweenSubdomains
    depends_on = subdomain1
    master_block = '1'
    paired_block = '0'
    new_boundary = 'master1_interface'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = '0'
  [../]


  [./v]
    order = FIRST
    family = LAGRANGE
    block = '1'
  [../]
[]

[Kernels]
  [./diff_u]
    type = CoeffParamDiffusion
    variable = u
    D = 4
    block = 0
  [../]
  [./diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 2
    block = 1
  [../]
[]

[InterfaceKernels]
  [./interface]
    type = InterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = master0_interface
    D = 4
    D_neighbor = 2
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  [../]
  [./right]
    type = DirichletBC
    variable = v
    boundary = 'right'
    value = 0
  [../]
  [./middle]
    type = MatchedValueBC
    variable = v
    boundary = 'master0_interface'
    v = u
  [../]
[]

[Preconditioning]
  [./fdp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]

[Debug]
  show_var_residual_norms = true
[]
