[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
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
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  [../]
  [./interface_again]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'primary1_interface'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff0]
    type = CoeffParamDiffusion
    variable = u
    D = 4
    block = 0
  [../]
  [./diff1]
    type = CoeffParamDiffusion
    variable = u
    D = 2
    block = 1
  [../]
[]

[InterfaceKernels]
  [./interface]
    type = InterfaceDiffusion
    variable = u
    neighbor_var = u
    boundary = primary0_interface
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
    variable = u
    boundary = 'right'
    value = 0
  [../]
[]

[Preconditioning]
  [./smp]
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
