[Mesh]
  # Build 2-by-2 mesh
  [mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    xmax = 2
    ny = 2
    ymax = 2
  []

  # Create blocs 0, 1, 2, 3
  [block_1]
    type = SubdomainBoundingBoxGenerator
    input = mesh
    block_id = 1
    bottom_left = '1 0 0'
    top_right = '2 1 0'
  []
  [block_2]
    type = SubdomainBoundingBoxGenerator
    input = block_1
    block_id = 2
    bottom_left = '0 1 0'
    top_right = '1 2 0'
  []
  [block_3]
    type = SubdomainBoundingBoxGenerator
    input = block_2
    block_id = 3
    bottom_left = '1 1 0'
    top_right = '2 2 0'
  []

  # Create inner sidesets
  [interface_01]
    type = SideSetsBetweenSubdomainsGenerator
    input = block_3
    primary_block = 0
    paired_block = 1
    new_boundary = 'interface_01'
  []
  [interface_13]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface_01
    primary_block = 1
    paired_block = 3
    new_boundary = 'interface_13'
  []
  [interface_32]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface_13
    primary_block = 3
    paired_block = 2
    new_boundary = 'interface_32'
  []
  [interface_20]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface_32
    primary_block = 2
    paired_block = 0
    new_boundary = 'interface_20'
  []

  # Create outer boundaries
  [boundary_left_0]
    type = SideSetsAroundSubdomainGenerator
    input = interface_20
    block = 0
    normal = '-1 0 0'
    new_boundary = 'left_0'
  []
  [boundary_bot_0]
    type = SideSetsAroundSubdomainGenerator
    input = boundary_left_0
    block = 0
    normal = '0 -1 0'
    new_boundary = 'bot_0'
  []
  [boundary_bot_1]
    type = SideSetsAroundSubdomainGenerator
    input = boundary_bot_0
    block = 1
    normal = '0 -1 0'
    new_boundary = 'bot_1'
  []
  [boundary_right_1]
    type = SideSetsAroundSubdomainGenerator
    input = boundary_bot_1
    block = 1
    normal = '1 0 0'
    new_boundary = 'right_1'
  []
  [boundary_right_3]
    type = SideSetsAroundSubdomainGenerator
    input = boundary_right_1
    block = 3
    normal = '1 0 0'
    new_boundary = 'right_3'
  []
  [boundary_top_3]
    type = SideSetsAroundSubdomainGenerator
    input = boundary_right_3
    block = 3
    normal = '0 1 0'
    new_boundary = 'top_3'
  []
  [boundary_top_2]
    type = SideSetsAroundSubdomainGenerator
    input = boundary_top_3
    block = 2
    normal = '0 1 0'
    new_boundary = 'top_2'
  []
  [boundary_left_2]
    type = SideSetsAroundSubdomainGenerator
    input = boundary_top_2
    block = 2
    normal = '-1 0 0'
    new_boundary = 'left_2'
  []
  uniform_refine = 4
[]

[Variables]
  # Need to have variable for each block to allow discontinuity
  [T0]
    block = 0
  []
  [T1]
    block = 1
  []
  [T2]
    block = 2
  []
  [T3]
    block = 3
  []
[]

[Kernels]
  # Diffusion kernel for each block's variable
  [diff_0]
    type = MatDiffusion
    variable = T0
    diffusivity = conductivity
    block = 0
  []
  [diff_1]
    type = MatDiffusion
    variable = T1
    diffusivity = conductivity
    block = 1
  []
  [diff_2]
    type = MatDiffusion
    variable = T2
    diffusivity = conductivity
    block = 2
  []
  [diff_3]
    type = MatDiffusion
    variable = T3
    diffusivity = conductivity
    block = 3
  []

  # Source for two of the blocks
  [source_0]
    type = BodyForce
    variable = T0
    value = 5e5
    block = '0'
  []
  [source_3]
    type = BodyForce
    variable = T3
    value = 5e5
    block = '3'
  []
[]

[InterfaceKernels]
  # Side set kernel to represent heat transfer across blocks
  # Automatically uses the materials defined in SideSetHeatTransferMaterial
  [gap_01]
    type = SideSetHeatTransferKernel
    # This variable defined on a given block must match the primary_block given when the side set was generated
    variable = T0
    # This variable defined on a given block must match the paired_block given when the side set was generated
    neighbor_var = T1
    boundary = 'interface_01'
  []
  [gap_13]
    type = SideSetHeatTransferKernel
    variable = T1
    neighbor_var = T3
    boundary = 'interface_13'
  []
  [gap_32]
    type = SideSetHeatTransferKernel
    variable = T3
    neighbor_var = T2
    boundary = 'interface_32'
  []
  [gap_20]
    type = SideSetHeatTransferKernel
    variable = T2
    neighbor_var = T0
    boundary = 'interface_20'
  []
[]

# Creating auxiliary variable to combine block restricted solutions
# Ignores discontinuity though
[AuxVariables]
  [T]
  []
[]

[AuxKernels]
  [temp_0]
    type = NormalizationAux
    variable = T
    source_variable = T0
    block = 0
  []
  [temp_1]
    type = NormalizationAux
    variable = T
    source_variable = T1
    block = 1
  []
  [temp_2]
    type = NormalizationAux
    variable = T
    source_variable = T2
    block = 2
  []
  [temp_3]
    type = NormalizationAux
    variable = T
    source_variable = T3
    block = 3
  []
[]

[BCs]
  # Boundary condition for each block's outer surface
  [bc_left_2]
    type = DirichletBC
    boundary = 'left_2'
    variable = T2
    value = 300.0
  []
  [bc_left_0]
    type = DirichletBC
    boundary = 'left_0'
    variable = T0
    value = 300.0
  []

  [bc_bot_0]
    type = DirichletBC
    boundary = 'bot_0'
    variable = T0
    value = 300.0
  []
  [bc_bot_1]
    type = DirichletBC
    boundary = 'bot_1'
    variable = T1
    value = 300.0
  []

  [./bc_top_2]
    type = ConvectiveFluxFunction # (Robin BC)
    variable = T2
    boundary = 'top_2'
    coefficient = 1e3 # W/K/m^2
    T_infinity = 600.0
  [../]
  [./bc_top_3]
    type = ConvectiveFluxFunction # (Robin BC)
    variable = T3
    boundary = 'top_3'
    coefficient = 1e3 # W/K/m^2
    T_infinity = 600.0
  [../]

  [./bc_right_3]
    type = ConvectiveFluxFunction # (Robin BC)
    variable = T3
    boundary = 'right_3'
    coefficient = 1e3 # W/K/m^2
    T_infinity = 600.0
  [../]
  [./bc_right_1]
    type = ConvectiveFluxFunction # (Robin BC)
    variable = T1
    boundary = 'right_1'
    coefficient = 1e3 # W/K/m^2
    T_infinity = 600.0
  [../]
[]

[Materials]
  [fuel]
    type = GenericConstantMaterial
    prop_names = 'conductivity'
    prop_values = 75
    block = '0 3'
  []
  [mod]
    type = GenericConstantMaterial
    prop_names = 'conductivity'
    prop_values = 7.5
    block = '1 2'
  []
  # Interface material used for SideSetHeatTransferKernel
  # Heat transfer meachnisms ignored if certain properties are not supplied
  [gap_mat]
    type = SideSetHeatTransferMaterial
    boundary = 'interface_01 interface_13 interface_32 interface_20'
    conductivity = 0.41
    gap_length = 0.002
    Tbulk = 750
    h_primary = 3000
    h_neighbor = 3000
    emissivity_primary = 0.85
    emissivity_neighbor = 0.85
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-12
  l_tol = 1e-8
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -ksp_gmres_restart'
  petsc_options_value = 'lu       superlu_dist                  50'
[]

[Outputs]
  exodus = true
[]
