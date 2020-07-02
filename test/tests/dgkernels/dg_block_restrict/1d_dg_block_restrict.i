[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 100
    xmax = 2
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.0 0 0'
    block_id = 1
    top_right = '2.0 0 0'
  [../]
  [./interface]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
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
    family = MONOMIAL
    block = 0
  [../]
  [./v]
    order = FIRST
    family = MONOMIAL
    block = 1
  [../]
[]

[Kernels]
  [./test_u]
    type = Diffusion
    variable = u
    block = 0
  [../]
  [./adv_u]
    type = ConservativeAdvection
    variable = u
    velocity = '1 0 0'
    block = 0
  [../]
  [./test_v]
    type = Diffusion
    variable = v
    block = 1
  [../]
  [./adv_v]
    type = ConservativeAdvection
    variable = v
    velocity = '1 0 0'
    block = 1
  [../]
[]

[DGKernels]
  [./dg_advection_u]
    type = DGConvection
    variable = u
    velocity = '1 0 0'
    block = 0
  [../]
  [./dg_diffusion_u]
    type = DGDiffusion
    variable = u
    sigma = 0
    epsilon = -1
    block = 0
  [../]
  [./dg_advection_v]
    type = DGConvection
    variable = v
    velocity = '1 0 0'
    block = 1
  [../]
  [./dg_diffusion_v]
    type = DGDiffusion
    variable = v
    sigma = 0
    epsilon = -1
    block = 1
  [../]
[]

[BCs]
  [./left]
    type = InflowBC
    variable = u
    boundary = 'left'
    inlet_conc = 2
    velocity = '1 0 0'
  [../]
  [./primary0_inteface]
    type = RobinBC
    variable = u
    boundary = 'primary0_interface'
  [../]
  [./primary1_interface]
   type = InflowBC
   variable = v
   boundary = 'primary1_interface'
   inlet_conc = 4
   velocity = '1 0 0'
  [../]
  [./right]
    type = RobinBC
    variable = v
    boundary = 'right'
  [../]
[]

[ICs]
  [./u_ic]
    type = ConstantIC
    variable = u
    value = 0
  [../]
  [./v_ic]
    type = ConstantIC
    variable = v
    value = 0
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
  nl_abs_tol = 1e-12
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

[Debug]
  show_var_residual_norms = true
[]
