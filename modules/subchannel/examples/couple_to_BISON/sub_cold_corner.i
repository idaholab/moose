rod_diameter = 0.00950
heated_length = 3.6
pitch = 0.0126
clad_top_gap_height = 0.00127
clad_bot_gap_height = 0.00127
clad_gap_width = 0.00009
clad_thickness = 0.00057
top_bot_clad_height = 0.00224
pellet_outer_radius = ${fparse rod_diameter/2.0 - clad_gap_width - clad_thickness}
pellet_height = ${fparse heated_length -  clad_top_gap_height - clad_bot_gap_height - 2.0 * top_bot_clad_height}
full_pin_power = 66600 #W
pin_factor = 0.7
pin_power = ${fparse full_pin_power * pin_factor}
T_in = 560.15
y_location = ${fparse 1.5 * pitch}
DY = ${fparse y_location + clad_bot_gap_height + top_bot_clad_height}

[Mesh]
  second_order = true
  [bisonMesh]
    type = SmearedPelletMeshGenerator
    clad_top_gap_height = ${clad_top_gap_height}
    clad_bot_gap_height = ${clad_bot_gap_height}
    top_bot_clad_height = ${top_bot_clad_height}
    clad_gap_width = ${clad_gap_width}
    clad_thickness = ${clad_thickness}
    ny_c =  98
    ny_p =  98
    nx_c = 4
    nx_p = 12
    pellet_height = ${fparse pellet_height}
    pellet_quantity = 1
    pellet_outer_radius =${pellet_outer_radius}
    pellet_mesh_density = customize
    clad_mesh_density = customize
  []
[]

[Functions]
  [volumetric_heat_rate]
    type = ParsedFunction
    value = '(P / (pi * R * R * L)) * (pi/2)*sin(pi * (y - DY) /L)'
    vars = 'L R P DY'
    vals = '${pellet_height} ${pellet_outer_radius} ${pin_power} ${DY}'
  []
[]

[DefaultElementQuality]
  failure_type = warning
[]

[Variables]
  [temperature]
    order = SECOND
    family = LAGRANGE
  []
[]

[AuxVariables]
  [Pin_surface_temperature]
  []
  [q_prime]
    order = CONSTANT
    family = MONOMIAL
  []
[]

# [AuxKernels]
#   [QPrime]
#     type = QPrimeAuxPin
#     diffusivity = 'thermal_conductivity'
#     rod_diameter = ${rod_diameter}
#     variable = q_prime
#     diffusion_variable = temperature
#     component = normal
#     boundary = clad_outside_right
#     execute_on = 'timestep_end'
#   []
# []

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = temperature
  []
  [heat_source]
    type = HeatSource
    variable = temperature
    function = volumetric_heat_rate
    block = pellet
  []
[]

[ThermalContact]
  [thermal_contact]
    # type = GapHeatTransfer
    type = GapHeatTransferLWR
    variable = temperature
    primary = pellet_outer_radial_surface
    secondary = clad_inside_right
    order = SECOND
    quadrature = true
    # gap_conductivity = 1
  []
[]

[Materials]
  [fuel_thermal]  # temperature and burnup dependent thermal properties of UO2 (BISON kernel)
    type = UO2Thermal
    block = pellet
    thermal_conductivity_model = NFIR
    temperature = temperature
    burnup_function = 0.0
  []
  [clad_thermal]
    block = clad
    type = ZryThermal
    temperature = temperature
  []
[]

[BCs]
  [pellet_centerline]
    type = NeumannBC
    variable = temperature
    boundary = 'centerline'
  []
  [pellet_outer_rad_surface]
    type = NeumannBC
    variable = temperature
    boundary = 'pellet_outer_radial_surface'
  []
  [clad_inner_surface]
    type = NeumannBC
    variable = temperature
    boundary = 'clad_inside_right'
  []
  [right]
    type = MatchedValueBC
    variable = temperature
    boundary = 'clad_outside_right'
    v = Pin_surface_temperature
  []
[]

[ICs]
  [temperature_ic]
    type = ConstantIC
    variable = temperature
    value = ${T_in}
  []
  [q_prime_ic]
    type = ConstantIC
    variable = q_prime
    value = 10.0
  []
[]

[UserObjects]
  [q_prime_uo]
    type = LayeredSideAverage
    boundary = clad_outside_right
    variable = q_prime
    num_layers = 1000
    direction = y
    execute_on = 'TIMESTEP_END'
  []
[]

[Problem]
  coord_type = RZ
  rz_coord_axis = Y
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'
  line_search = 'none'

  [Quadrature]
    order = FIFTH
    side_order = SEVENTH
  []
[]

[Outputs]
  exodus = true
[]
