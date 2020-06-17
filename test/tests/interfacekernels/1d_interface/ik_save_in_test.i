[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
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
    master_block = '0'
    paired_block = '1'
    new_boundary = 'master0_interface'
  [../]
  [./interface_again]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface
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

[AuxVariables]
  [./master_resid]
  [../]
  [./secondary_resid]
  [../]
  [./master_jac]
  [../]
  [./secondary_jac]
  [../]
[]

[Kernels]
  [./diff_u]
    type = CoeffParamDiffusion
    variable = u
    D = 4
    block = 0
    save_in = 'master_resid'
  [../]
  [./diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 2
    block = 1
    save_in = 'secondary_resid'
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
    save_in_var_side = 'm s'
    save_in = 'master_resid secondary_resid'
    diag_save_in_var_side = 'm s'
    diag_save_in = 'master_jac secondary_jac'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 0
    save_in = 'master_resid'
  [../]
  [./right]
    type = DirichletBC
    variable = v
    boundary = 'right'
    value = 1
    save_in = 'secondary_resid'
  [../]
  [./middle]
    type = MatchedValueBC
    variable = v
    boundary = 'master0_interface'
    v = u
    save_in = 'secondary_resid'
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
