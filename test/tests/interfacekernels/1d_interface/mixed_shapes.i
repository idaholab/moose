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
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  [../]
  [./interface_again]
    input = interface
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'primary1_interface'
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
    family = MONOMIAL
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
  [./body_u]
    type = BodyForce
    variable = u
    block = 0
    function = 'x^3+x^2+x+1'
  [../]
  [./body_v]
    type = BodyForce
    variable = v
    block = 1
    function = 'x^3+x^2+x+1'
  [../]
[]

[DGKernels]
  [./dg_diff_v]
    type = DGDiffusion
    variable = v
    block = 1
    diff = 2
    sigma = 6
    epsilon = -1
  [../]
[]

[InterfaceKernels]
  [./interface]
    type = OneSideDiffusion
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    D = 4
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  [../]
  # [./right]
  #   type = DirichletBC
  #   variable = v
  #   boundary = 'right'
  #   value = 0
  # [../]
  [./right]
    type = DGFunctionDiffusionDirichletBC
    variable = v
    boundary = 'right'
    function = 0
    epsilon = -1
    sigma = 6
  [../]
  [./middle]
    type = NeumannBC
    variable = u
    boundary = 'primary0_interface'
    value = '.5'
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
