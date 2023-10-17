outer_htc = 10 # W/m^2/K
outer_Tinf = 300 # K

[GlobalParams]
  order = SECOND
  family = LAGRANGE
[]

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
[]

[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 6
    nz = 6
    xmin = -1
    xmax = -0.5
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
    elem_type = HEX27
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3 4 5'
    new_boundary = 'left_bottom left_back left_right left_front left_left left_top'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  []

  [right_block]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 4
    ny = 8
    nz = 8
    xmin = 0.5
    xmax = 1
    ymin = -0.5
    ymax = 0.5
    zmin = -0.5
    zmax = 0.5
    elem_type = HEX27
  []
  [right_block_sidesets]
    type = RenameBoundaryGenerator
    input = right_block
    old_boundary = '0 1 2 3 4 5'
    # new_boundary = 'right_bottom right_back right_right right_front right_left right_top'
    new_boundary = '100 101 102 103 104 105'
  []
  [right_block_sidesets_rename]
    type = RenameBoundaryGenerator
    input = right_block_sidesets
    old_boundary = '100 101 102 103 104 105'
    new_boundary = 'right_bottom right_back right_right right_front right_left right_top'
  []
  [right_block_id]
    type = SubdomainIDGenerator
    input = right_block_sidesets_rename
    subdomain_id = 2
  []

  [combined_mesh]
    type = MeshCollectionGenerator
    inputs = 'left_block_id right_block_id'
  []

  [left_lower]
    type = LowerDBlockFromSidesetGenerator
    input = combined_mesh
    sidesets = 'left_right'
    new_block_id = '10001'
    new_block_name = 'secondary_lower'
  []
  [right_lower]
    type = LowerDBlockFromSidesetGenerator
    input = left_lower
    sidesets = 'right_left'
    new_block_id = '10000'
    new_block_name = 'primary_lower'
  []
[]

[Functions]
  [temp]
    type = PiecewiseLinear
    x = '0   1'
    y = '100 200'
  []
[]

[Variables]
  [temp]
    initial_condition = 500
  []
  [lm]
    order = SECOND
    family = LAGRANGE
    block = 'secondary_lower'
  []
[]

[AuxVariables]
  [power_density]
    block = 1
    initial_condition = 50e3
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temp
    block = '1 2'
  []
  [heat_source]
    type = CoupledForce
    variable = temp
    block = '1'
    v = power_density
  []
[]

[Materials]
  [heat1]
    type = HeatConductionMaterial
    block = '1 2'
    specific_heat = 1.0
    thermal_conductivity = 34.6
  []
[]

[UserObjects]
  [radiation]
    type = GapFluxModelRadiation
    temperature = temp
    boundary = 'left_right'
    primary_emissivity = 0.0
    secondary_emissivity = 0.0
  []
  [conduction]
    type = GapFluxModelConduction
    temperature = temp
    boundary = 'left_right'
    gap_conductivity = 5.0
  []
[]

[Constraints]
  [ced]
    type = ModularGapConductanceConstraint
    variable = lm
    secondary_variable = temp
    primary_boundary = 'right_left'
    primary_subdomain = 'primary_lower'
    secondary_boundary = 'left_right'
    secondary_subdomain = 'secondary_lower'
    gap_flux_models = 'radiation conduction'
    gap_geometry_type = PLATE
  []
[]

[BCs]
  [RPV_out_BC] # k \nabla T = h (T- T_inf) at RPV outer boundary
    type = ConvectiveFluxFunction # (Robin BC)
    variable = temp
    boundary = 'right_right' # outer RPV
    coefficient = ${outer_htc}
    T_infinity = ${outer_Tinf}
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'

  dt = 1
  dtmin = 0.01
  end_time = 1

  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-8
[]

[Outputs]
  exodus = true
  csv = true
  [Console]
    type = Console
  []
[]

[Postprocessors]
  [temp_left]
    type = SideAverageValue
    boundary = 'left_right'
    variable = temp
  []

  [temp_right]
    type = SideAverageValue
    boundary = 'right_left'
    variable = temp
  []

  [flux_left]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 'left_right'
    diffusivity = thermal_conductivity
  []
  [flux_right]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 'right_left'
    diffusivity = thermal_conductivity
  []
  [ptot]
    type = ElementIntegralVariablePostprocessor
    variable = power_density
    block = 1
  []
  [convective_out]
    type = ConvectiveHeatTransferSideIntegral
    T_solid = temp
    boundary = 'right_right' # outer RVP
    T_fluid = ${outer_Tinf}
    htc = ${outer_htc}
  []
  [heat_balance] # should be equal to 0 upon convergence
    type = ParsedPostprocessor
    function = '(convective_out - ptot) / ptot'
    pp_names = 'convective_out ptot'
  []
[]

[VectorPostprocessors]
  [NodalTemperature]
    type = NodalValueSampler
    sort_by = id
    boundary = 'left_right right_left'
    variable = temp
  []
[]
