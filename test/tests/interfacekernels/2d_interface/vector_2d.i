[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  xmax = 2
  ny = 2
  ymax = 2
  elem_type = QUAD9
[]

[MeshModifiers]
  [./subdomain1]
    type = SubdomainBoundingBox
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]
  [./break_boundary]
    type = BreakBoundaryOnSubdomain
    depends_on = subdomain1
  [../]
  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = break_boundary
    master_block = '0'
    paired_block = '1'
    new_boundary = 'master0_interface'
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
    block = 0
  [../]
  [./curl_v_plus_v]
    type = VectorFEWave
    variable = v
    block = 1
  [../]
  [./u_source]
    type = VectorBodyForce
    variable = u
    function_x = 1
    function_y = 1
    function_z = 1
  [../]
[]

[InterfaceKernels]
  [./parallel]
    type = VectorPenaltyInterfaceDiffusion
    variable = u
    neighbor_var = v
    boundary = master0_interface
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
