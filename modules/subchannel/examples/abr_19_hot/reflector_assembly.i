###################################################
# Thermal-hydraulics parameters
###################################################
T_in = 653.15
P_out = 758423 # Pa
reactor_power = 1000e6 #WTh
fuel_assemblies_per_power_unit = '${fparse 180}'
#fuel_pins_per_assembly = 271
flow_area = 0.00678363
#pin_power = ${fparse reactor_power/(fuel_assemblies_per_power_unit*fuel_pins_per_assembly)} # Approx.
mass_flux_in = '${fparse 1256*4/fuel_assemblies_per_power_unit/flow_area/10}' # kg/(m2.s)

###################################################
# Geometric parameters
###################################################
#f = ${fparse sqrt(3) / 2}

# units are cm - do not forget to convert to meter
scale_factor = 0.01
fuel_element_pitch = '${fparse 16.2471*scale_factor}'
inter_assembly_gap = '${fparse 0.4348*scale_factor}'
duct_thickness = '${fparse 0.3966*scale_factor}'
fuel_pin_pitch = '${fparse 1.5528*scale_factor}' #'${fparse 1.541*scale_factor}' #'${fparse 1.5528*scale_factor}'
fuel_pin_diameter = '${fparse 1.5514*scale_factor}' #'${fparse 1.539*scale_factor}' #'${fparse 1.5514*scale_factor}'
# wire_z_spacing = '${fparse 20.32*scale_factor}' ###
# wire_diameter = '${fparse 0.1307*scale_factor}'
n_rings = 6
length_entry_fuel = '${fparse 160.92*scale_factor}'
length_heated_fuel = '${fparse 85.82*scale_factor}'
length_outlet_fuel = '${fparse 233.46*scale_factor}' #236
#height = ${fparse length_entry_fuel+length_heated_fuel+length_outlet_fuel}
orifice_plate_height = '${fparse 5*scale_factor}'
duct_outside = '${fparse fuel_element_pitch - inter_assembly_gap}'
duct_inside = '${fparse duct_outside - 2 * duct_thickness}'
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = '${fparse n_rings}'
    n_cells = 50 #100
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pin_diameter = '${fparse fuel_pin_diameter}'
    pitch = '${fparse fuel_pin_pitch}'
    dwire = '0' #'${fparse wire_diameter}'
    hwire = '0' #'${fparse wire_z_spacing}'
    spacer_z = '${fparse orifice_plate_height} ${fparse length_entry_fuel}'
    spacer_k = '0 0' #'0.5 0.5'
  []

  #  [fuel_pins]
  #    type = TriPinMeshGenerator
  #    input = subchannel
  #    nrings = '${fparse n_rings}'
  #    n_cells = 100
  #    unheated_length_entry = '${fparse length_entry_fuel}'
  #    heated_length = '${fparse length_heated_fuel}'
  #    unheated_length_exit = '${fparse length_outlet_fuel}'
  #    pitch = '${fparse fuel_pin_pitch}'
  #  []
  #
  [duct]
    type = SCMTriDuctMeshGenerator
    input = subchannel #fuel_pins
    nrings = '${fparse n_rings}'
    n_cells = 50 #100
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pitch = '${fparse fuel_pin_pitch}'
  []
[]

[Functions]
  #  [axial_heat_rate]
  #    type = ParsedFunction
  #    value = 'if(z>l1 & z<l2, 1.0, 1.0)'
  #    #'(pi/2)*sin(pi*z/L)'
  #    vars = 'l1 l2'
  #    vals = '${length_entry_fuel} ${fparse length_entry_fuel+length_heated_fuel}'
  #  []
  [axial_heat_rate]
    type = ParsedFunction
    value = '(pi/2)*sin(pi*z/L)'
    vars = 'L'
    vals = '${length_heated_fuel}'
  []
[]

[AuxVariables]
  [mdot]
    block = subchannel
  []
  [SumWij]
    block = subchannel
  []
  [P]
    block = subchannel
  []
  [DP]
    block = subchannel
  []
  [h]
    block = subchannel
  []
  [T]
    block = subchannel
  []
  [rho]
    block = subchannel
  []
  [S]
    block = subchannel
  []
  [Sij]
    block = subchannel
  []
  [w_perim]
    block = subchannel
  []
  [q_prime]
    block = subchannel
  []
  [displacement]
    block = subchannel
  []
  [mu]
    block = subchannel
  []
  [q_prime_duct]
    block = duct
  []
  [Tduct]
    block = duct
    initial_condition = 653.15
  []
[]

[FluidProperties]
  [sodium]
    type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = TriSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  P_out = ${P_out}
  CT = 1.0
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-5
  T_tol = 1.0e-3
  implicit = true
  segregated = false
  staggered_pressure = false
  monolithic_thermal = false
  verbose_multiapps = true
  verbose_subchannel = true #false
[]

[ICs]
  [S_IC]
    type = TriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = TriWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = TriPowerIC
    variable = q_prime
    power = '${fparse 0.01*reactor_power/fuel_assemblies_per_power_unit}' # W
    filename = "pin_power_profile_91.txt"
    axial_heat_rate = axial_heat_rate
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [P_ic]
    type = ConstantIC
    variable = P
    value = 0.0
  []

  [DP_ic]
    type = ConstantIC
    variable = DP
    value = 0.0
  []

  [Viscosity_ic]
    type = ViscosityIC
    variable = mu
    p = ${P_out}
    T = T
    fp = sodium
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = sodium
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = sodium
  []

  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []
[]

[AuxKernels]
  [P_out_bc]
    type = ConstantAux
    variable = P
    boundary = outlet
    value = ${P_out}
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [mdot_in_bc]
    type = MassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
    block = subchannel
  []
[]

#[UserObjects]
#  [Tpin_avg_uo]
#    type = NearestPointLayeredAverage
#    direction = z
#    num_layers = 1000
#    variable = Tpin
#    block = fuel_pins
#    points = '${fparse 0.012} 0.0 0.0'
#    execute_on = timestep_end
#  []
#[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 30
  fixed_point_min_its = 1
  fixed_point_rel_tol = 1e-6
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################
[MultiApps]
  #  # Multiapp to pin heat conduction module
  #  [pin_map]
  #    type = FullSolveMultiApp
  #    input_files = pin.i # seperate file for multiapps due to radial power profile
  #    execute_on = 'timestep_end'
  #    positions = '0  0   0'
  #    bounding_box_padding = '0 0 0.01'
  #  []

  # Multiapp to duct heat conduction module
  #  [duct_map]
  #    type = FullSolveMultiApp
  #    input_files = wrapper.i # seperate file for duct heat conduction
  #    execute_on = 'timestep_end'
  #    positions = '0   0   0'
  #    bounding_box_padding = '0.0 0.0 0.01'
  #  []

  # [viz]
  #   type = FullSolveMultiApp
  #   input_files = "3d_reflector.i"
  #   execute_on = "final"
  #   output_in_position = true
  # []
[]

[Transfers]

  # [Tpin] # send pin surface temperature to bison,
  #   type = MultiAppInterpolationTransfer
  #   to_multi_app = pin_map
  #   source_variable = Tpin
  #   variable = Pin_surface_temperature
  # []
  # [q_prime_pin] # send heat flux from slave/BISON/heatConduction to subchannel/master
  #   type = MultiAppInterpolationTransfer
  #   from_multi_app = pin_map
  #   source_variable = q_prime_pin
  #   variable = q_prime
  # []

  #  [duct_temperature_transfer] # Send duct temperature to heat conduction
  #    type = MultiAppInterpolationTransfer
  #    to_multi_app = duct_map
  #    source_variable = Tduct
  #    variable = duct_surface_temperature
  #  []
  #  [q_prime_duct] # Recover q_prime from heat conduction solve
  #    type = MultiAppInterpolationTransfer
  #    from_multi_app = duct_map
  #    source_variable = q_prime_d
  #    variable = q_prime_duct
  #  []

  # [subchannel_transfer]
  #   type = SCMSolutionTransfer
  #   to_multi_app = viz
  #   variable = 'mdot SumWij P DP h T rho mu S q_prime'
  # []
  # [pin_transfer]
  #   type = MultiAppDetailedPinSolutionTransfer
  #   to_multi_app = viz
  #   variable = 'Tpin q_prime'
  # []
[]
