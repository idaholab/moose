rpv_core_gap_size = 0.2

core_outer_radius = 2
rpv_inner_radius = '${fparse 2 + rpv_core_gap_size}'
rpv_outer_radius = '${fparse 2.5 + rpv_core_gap_size}'
rpv_width = '${fparse rpv_outer_radius - rpv_inner_radius}'

rpv_outer_htc = 10 # W/m^2/K
rpv_outer_Tinf = 300 # K

core_blocks = '1'
rpv_blocks = '3'

[Mesh]
  [gmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '${core_outer_radius} ${rpv_core_gap_size} ${rpv_width}'
    ix = '400 1 100'
    dy = 1
    iy = '5'
  []
  [set_block_id1]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    bottom_left = '0 0 0'
    top_right = '${core_outer_radius} 1 0'
    block_id = 1
    location = INSIDE
  []
  [rename_core_bdy]
    type = SideSetsBetweenSubdomainsGenerator
    input = set_block_id1
    primary_block = 1
    paired_block = 0
    new_boundary = 'core_outer'
  []
  [set_block_id3]
    type = SubdomainBoundingBoxGenerator
    input = rename_core_bdy
    bottom_left = '${rpv_inner_radius} 0 0'
    top_right = '${rpv_outer_radius} 1 0'
    block_id = 3
    location = INSIDE
  []
  [rename_inner_rpv_bdy]
    type = SideSetsBetweenSubdomainsGenerator
    input = set_block_id3
    primary_block = 3
    paired_block = 0
    new_boundary = 'rpv_inner'
  []
  # comment out for test without gap
  [2d_mesh]
    type = BlockDeletionGenerator
    input = rename_inner_rpv_bdy
    block = 0
  []
  allow_renumbering = false
[]

[Problem]
  coord_type = RZ
[]

[Variables]
  [Tsolid]
    initial_condition = 500
  []
[]

[Kernels]
  [heat_source]
    type = CoupledForce
    variable = Tsolid
    block = '${core_blocks}'
    v = power_density
  []
  [heat_conduction]
    type = HeatConduction
    variable = Tsolid
  []
[]

[BCs]
  [RPV_out_BC] # k \nabla T = h (T- T_inf) at RPV outer boundary
    type = ConvectiveFluxFunction # (Robin BC)
    variable = Tsolid
    boundary = 'right' # outer RPV
    coefficient = ${rpv_outer_htc}
    T_infinity = ${rpv_outer_Tinf}
  []
[]

[ThermalContact]
  [RPV_gap]
    type = GapHeatTransfer
    gap_geometry_type = 'CYLINDER'
    emissivity_primary = 0.8
    emissivity_secondary = 0.8
    variable = Tsolid
    primary = 'core_outer'
    secondary = 'rpv_inner'
    gap_conductivity = 0.1
    quadrature = true
  []
[]

[AuxVariables]
  [power_density]
    block = '${core_blocks}'
    initial_condition = 50e3
  []
[]

[Materials]
  [simple_mat]
    type = HeatConductionMaterial
    thermal_conductivity = 34.6 # W/m/K
  []
[]

[Postprocessors]
  [Tcore_avg]
    type = ElementAverageValue
    variable = Tsolid
    block = '${core_blocks}'
  []
  [Tcore_max]
    type = ElementExtremeValue
    value_type = max
    variable = Tsolid
    block = '${core_blocks}'
  []
  [Tcore_min]
    type = ElementExtremeValue
    value_type = min
    variable = Tsolid
    block = '${core_blocks}'
  []
  [Trpv_avg]
    type = ElementAverageValue
    variable = Tsolid
    block = '${rpv_blocks}'
  []
  [Trpv_max]
    type = ElementExtremeValue
    value_type = max
    variable = Tsolid
    block = '${rpv_blocks}'
  []
  [Trpv_min]
    type = ElementExtremeValue
    value_type = min
    variable = Tsolid
    block = '${rpv_blocks}'
  []
  [ptot]
    type = ElementIntegralVariablePostprocessor
    variable = power_density
    block = '${core_blocks}'
  []
  [rpv_convective_out]
    type = ConvectiveHeatTransferSideIntegral
    T_solid = Tsolid
    boundary = 'right' # outer RVP
    T_fluid = ${rpv_outer_Tinf}
    htc = ${rpv_outer_htc}
  []
  [heat_balance] # should be equal to 0 upon convergence
    type = ParsedPostprocessor
    function = '(rpv_convective_out - ptot) / ptot'
    pp_names = 'rpv_convective_out ptot'
  []
  [flux_from_core] # converges to ptot as the mesh is refined
    type = SideDiffusiveFluxIntegral
    variable = Tsolid
    boundary = core_outer
    diffusivity = thermal_conductivity
  []
  [flux_into_rpv] # converges to rpv_convective_out as the mesh is refined
    type = SideDiffusiveFluxIntegral
    variable = Tsolid
    boundary = rpv_inner
    diffusivity = thermal_conductivity
  []
[]

[VectorPostprocessors]
  [NodalTemperature]
    type = NodalValueSampler
    sort_by = id
    boundary = 'rpv_inner core_outer'
    variable = Tsolid
  []
[]

[Executioner]
  type = Steady
  automatic_scaling = true
  compute_scaling_once = false
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart '
  petsc_options_value = 'hypre boomeramg 100'
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-10
  l_max_its = 100
  [Quadrature]
    # order = fifth
    side_order = seventh
  []
  line_search = none
[]

[Outputs]
  exodus = false
  csv = true
[]
