[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    xmax = 2
    ny = 2
    ymax = 2
    elem_type = QUAD9
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]
  [./break_boundary]
    type = BreakBoundaryOnSubdomainGenerator
    input = subdomain1
  [../]
  [./interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = break_boundary
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = NEDELEC_ONE
    block = 0
  [../]

  [./v]
    order = FIRST
    family = NEDELEC_ONE
    block = 1
  [../]
[]

[Kernels]
  [./curl_u_plus_u]
    type = VectorFEWave
    variable = u
    x_forcing_func = 1
    y_forcing_func = 1
    z_forcing_func = 1
    block = 0
  [../]
  [./curl_v_plus_v]
    type = VectorFEWave
    variable = v
    block = 1
  [../]
[]

[InterfaceKernels]
  [./parallel]
    type = VectorPenaltyInterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    penalty = 1e6
  [../]
[]

[BCs]
  # Natural condition of VectorFEWave weak form is curl(u) = 0, curl(v) = 0
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
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]
