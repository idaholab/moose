# a wrapper mesh for coupling to subchannel

# sqrt(3) / 2 is by how much flat to flat is smaller than corer to corner
f = '${fparse sqrt(3) / 2}'

# units are meters
scale_factor = 0.01
length_entry_fuel = '${fparse 60*scale_factor}'
length_heated_fuel = '${fparse 80*scale_factor}'
length_outlet_fuel = '${fparse 120*scale_factor}'
height = '${fparse length_entry_fuel+length_heated_fuel+length_outlet_fuel}'
fuel_element_pitch = '${fparse 14.598*scale_factor}'
inter_assembly_gap = '${fparse 0.4*scale_factor}'
wrapper_thickness = '${fparse 0.3*scale_factor}'
duct_outside = '${fparse fuel_element_pitch - inter_assembly_gap}'
duct_inside = '${fparse duct_outside - 2 * wrapper_thickness}'

# number of radial elements in the wrapper
n_radial = 4

# number of azimuthal elements per side
n_az = 4

# number of axial elements
n_ax = 10

# System variables
T_in = 630

[Mesh]
  [2d_fuel_element]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '${n_az} ${n_az} ${n_az} ${n_az} ${n_az} ${n_az}'
    background_intervals = 1
    background_block_ids = '1'
    # note that polygon_size is "like radius"
    polygon_size = '${fparse duct_outside / 2}'
    duct_sizes = '${fparse duct_inside / 2 / f}'
    duct_intervals = '${n_radial}'
    duct_block_ids = '2'
    interface_boundary_names = 'inside'
    external_boundary_name = 'outside'
  []

  [extrude]
    type = FancyExtruderGenerator
    direction = '0 0 1'
    input = 2d_fuel_element
    heights = '${height}'
    num_layers = '${n_ax}'
  []

  [inlet_boundary]
    type = ParsedGenerateSideset
    input = extrude
    combinatorial_geometry = 'z < 1e-6'
    normal = '0 0 -1'
    new_sideset_name = 'inlet'
  []

  [outlet_boundary]
    type = ParsedGenerateSideset
    input = inlet_boundary
    combinatorial_geometry = 'z > ${fparse height - 1e-6}'
    normal = '0 0 1'
    new_sideset_name = 'outlet'
  []

  [inside_boundary]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 2
    paired_block = 1
    new_boundary = 'inside'
    input = outlet_boundary
  []

  [remove]
    type = BlockDeletionGenerator
    block = 1
    input = inside_boundary
  []

  [rename]
    type = RenameBlockGenerator
    input = remove
    old_block = '2'
    new_block = 'wrapper'
  []

  [rotate]
    type = TransformGenerator
    input = rename
    transform = ROTATE
    vector_value = '30 0 0'
  []
[]

[Functions]
  [volumetric_heat_rate]
    type = ParsedFunction
    value = '1.0*H'
    vars = 'H'
    vals = '${height}'
  []
  [duct_outside_temperature]
    type = ParsedFunction
    value = '630+(736-630)*z/L'
    vars = 'L'
    vals = '${height}'
  []
[]

[Variables]
  [temperature]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
  [heat_source]
    type = HeatSource
    variable = temperature
    function = volumetric_heat_rate
  []
[]

[AuxVariables]
  [q_prime_d]
    order = CONSTANT
    family = MONOMIAL
  []
  [q_prime_d_out]
    order = CONSTANT
    family = MONOMIAL
  []
  [duct_surface_temperature]
  []
  [duct_surface_temperature_out]
  []
[]

[AuxKernels]
  [QPrime]
    type = SCMTriDuctQPrimeAux
    diffusivity = 'thermal_conductivity'
    flat_to_flat = '${fparse duct_inside}'
    variable = q_prime_d
    diffusion_variable = temperature
    component = normal
    boundary = 'inside'
    execute_on = 'timestep_end'
  []
  [QPrime_duct_out]
    type = DiffusionFluxAux
    diffusivity = 'thermal_conductivity'
    variable = q_prime_d_out
    diffusion_variable = temperature
    component = normal
    boundary = 'outside'
    execute_on = 'timestep_end'
  []
[]

[Materials]
  [heat_conductor]
    type = HeatConductionMaterial
    thermal_conductivity = 1.0
    block = wrapper
  []
[]

[BCs]
  [isolated_bc]
    type = NeumannBC
    variable = temperature
    boundary = 'inlet outlet'
  []
  [inside_bc]
    type = MatchedValueBC
    variable = temperature
    boundary = 'inside'
    v = duct_surface_temperature
  []
  [outside_bc]
    type = FunctionDirichletBC
    variable = temperature
    boundary = 'outside'
    function = duct_outside_temperature
  []
  # [outside_bc]
  #   type = MatchedValueBC
  #   variable = temperature
  #   boundary = 'inside'
  #   v = duct_surface_temperature_out
  # []
[]

[ICs]
  [temperature_ic]
    type = ConstantIC
    variable = temperature
    value = ${T_in}
  []
  [q_prime_ic]
    type = ConstantIC
    variable = q_prime_d
    value = 0.0
  []
  [temperature_duct_in_ic]
    type = ConstantIC
    variable = duct_surface_temperature
    value = ${T_in}
  []
  [temperature_duct_out_ic]
    type = ConstantIC
    variable = duct_surface_temperature_out
    value = ${T_in}
  []
[]

[UserObjects]
  [q_prime_uo]
    type = LayeredSideAverage
    boundary = 'inside'
    variable = q_prime_d
    num_layers = 1000
    direction = z
    execute_on = 'TIMESTEP_END'
  []
[]

[Problem]
  coord_type = XYZ
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'

  [Quadrature]
    order = THIRD
    side_order = FOURTH
  []
[]

# [MultiApps]
#   # Multiapp to pin heat conduction module
#   [external_duct_map]
#     type = FullSolveMultiApp
#     input_files = core.i # seperate file for multiapps due to radial power profile
#     execute_on = 'timestep_end'
#     positions = '0  0   0'
#     bounding_box_padding = '0 0 0.01'
#   []
# []
#
# [Transfers]
#   [q_prime_duct_out_to_core] # Send duct temperature to heat conduction
#     type = MultiAppInterpolationTransfer
#     to_multi_app = external_duct_map
#     source_variable = q_prime_d_out
#     variable = q_prime_duct
#   []
#   [T_duct_out_from_core] # Recover q_prime from heat conduction solve
#     type = MultiAppInterpolationTransfer
#     from_multi_app = external_duct_map
#     source_variable = T_wrapper
#     variable = duct_surface_temperature_out
#   []
# []

[Outputs]
  exodus = true
[]
