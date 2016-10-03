[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  xmax = 2
  ny = 2
  ymax = 2
[]

[MeshModifiers]
  [./subdomain1]
    type = SubdomainBoundingBox
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]
  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = subdomain1
    master_block = '0'
    paired_block = '1'
    new_boundary = 'master0_interface'
  [../]
  [./split_boundary]
    depends_on = interface
    type = AddSideSetsForSplitBoundary
    subdomain_blocks = '0; 1'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]


  [./v]
    order = FIRST
    family = LAGRANGE
    block = 1
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
  [./source_u]
    type = BodyForce
    variable = u
    value = 1
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
  [./u]
    type = VacuumBC
    variable = u
    boundary = 'left_subdomain-0_bnd bottom_subdomain-0_bnd right top'
  [../]
  [./v]
    type = VacuumBC
    variable = v
    boundary = 'left_subdomain-1_bnd bottom_subdomain-1_bnd'
  [../]
  [./middle]
    type = MatchedValueBC
    variable = v
    boundary = 'master0_interface'
    v = u
  [../]
[]

[Postprocessors]
  [./u_int]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = 0
  [../]
  [./v_int]
    type = ElementIntegralVariablePostprocessor
    variable = v
    block = 1
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
