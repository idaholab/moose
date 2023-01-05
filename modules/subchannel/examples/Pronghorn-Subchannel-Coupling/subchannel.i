#
# Problems to be fixed:
# 1. subchannel does not like to be subapp
# 2. parallel execution of subchannel
# 3. The coupling to FV is very awkward because the machinery
#    is designed for FEM. See 7_assemblies_plus_subchannel.i input.
#

# Following Advanced Burner Test Reactor Preconceptual Design Report
# Vailable at: https://www.ne.anl.gov/eda/ABTR_1cv2_ws.pdf
###################################################
# Thermal-hydraulics parameters
###################################################
T_in = 628.15
P_out = 758423 # Pa
reactor_power = 500e6 #WTh
fuel_assemblies_per_power_unit = ${fparse 2.5}
fuel_pins_per_assembly = 217
pin_power = ${fparse reactor_power/(fuel_assemblies_per_power_unit*fuel_pins_per_assembly)} # Approx.
mass_flux_in = ${fparse 2786} # kg/(m2.s)

###################################################
# Geometric parameters
###################################################
f = ${fparse sqrt(3) / 2}

# units are cm - do not forget to convert to meter
scale_factor = 0.01
fuel_element_pitch = ${fparse 14.598*scale_factor}
inter_assembly_gap = ${fparse 0.4*scale_factor}
duct_thickness = ${fparse 0.3*scale_factor}
fuel_pin_pitch = ${fparse 0.904*scale_factor}
fuel_pin_diameter= ${fparse 0.8*scale_factor}
wire_z_spacing = ${fparse 20.32*scale_factor}
wire_diameter = ${fparse 0.103*scale_factor}
n_rings = 9
length_entry_fuel = ${fparse 60*scale_factor}
length_heated_fuel = ${fparse 80*scale_factor}
length_outlet_fuel = ${fparse 120*scale_factor}
height = ${fparse length_entry_fuel+length_heated_fuel+length_outlet_fuel}
orifice_plate_height = ${fparse 5*scale_factor}
duct_outside = ${fparse fuel_element_pitch - inter_assembly_gap}
duct_inside = ${fparse duct_outside - 2 * duct_thickness}
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = '${fparse n_rings}'
    n_cells = 26
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    rod_diameter = '${fparse fuel_pin_diameter}'
    pitch = '${fparse fuel_pin_pitch}'
    dwire = '${fparse wire_diameter}'
    hwire = '${fparse wire_z_spacing}'
    spacer_z = '${fparse orifice_plate_height} ${fparse length_entry_fuel}'
    spacer_k = '0.5 0.5'
  []

  [fuel_pins]
    type = TriPinMeshGenerator
    input = subchannel
    nrings = '${fparse n_rings}'
    n_cells = 26
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pitch = '${fparse fuel_pin_pitch}'
  []

  [duct]
    type = TriDuctMeshGenerator
    input = fuel_pins
    nrings = '${fparse n_rings}'
    n_cells = 26
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse length_entry_fuel}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse length_outlet_fuel}'
    pitch = '${fparse fuel_pin_pitch}'
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    value = 'if(z>l1 & z<l2, sin(pi * (z - l1) / (l2 - l1)), 0.0)'
    #'(pi/2)*sin(pi*z/L)'
    vars = 'l1 l2'
    vals = '${length_entry_fuel} ${fparse length_entry_fuel+length_heated_fuel}'
  []

  [dt_fn]
    type = PiecewiseLinear
    x = '0   2   10 20 100 1000'
    y = '0.5 0.5 2  10 100 500'
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
  [Tpin]
    block = fuel_pins
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
    block = fuel_pins
  []
  [mu]
    block = subchannel
  []
  [q_prime_duct]
    block = duct
    initial_condition = 0
  []
  [Tduct]
    block = duct
  []
[]

[Modules]
  [FluidProperties]
    [sodium]
       type = PBSodiumFluidProperties
    []
  []
[]

[Problem]
  type = LiquidMetalSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 25
  beta = 0.006
  P_out = ${P_out}
  CT = 2.6
  enforce_uniform_pressure = false
  compute_density = true
  compute_viscosity = true
  compute_power = false
  P_tol = 1.0e-3
  T_tol = 1.0e-3
  implicit = false
  segregated = true
  staggered_pressure = false
  monolithic_thermal = false
  verbose_multiapps = true
  verbose_subchannel = false
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
    power = 0.0 #${pin_power} # W
    filename = "pin_power_profile217.txt"
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
    value = ${P_out}
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
    p = P
    T = T
    fp = sodium
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = P
    T = T
    fp = sodium
  []

  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []

  [T_duct_ic]
    type = ConstantIC
    variable = Tduct
    value = ${T_in}
  []
[]

[AuxKernels]
  [P_out_bc]
    type = ConstantAux
    variable = P
    boundary = outlet
    value = report_pressure_outlet
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
    mass_flux = report_mass_flux_inlet
    execute_on = 'timestep_begin'
    block = subchannel
  []
[]

[Outputs]
  exodus = true
[]

[Postprocessors]
  [total_pressure_drop]
    type = TriSubChannelPressureDrop
    execute_on = "timestep_end"
  []

  [report_mass_flux_inlet]
    type = Receiver
    default = ${mass_flux_in}
  []

  [report_pressure_outlet]
    type = Receiver
    default = ${P_out}
  []
[]

# [Executioner]
#   type = Transient
#   [TimeStepper]
#     type = FunctionDT
#     function = dt_fn
#   []
#   end_time = 1e4
#   petsc_options_iname = '-pc_type -pc_hypre_type'
#   petsc_options_value = 'hypre boomeramg'
# []

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 2
  fixed_point_min_its = 1
  fixed_point_rel_tol = 1e-6
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################
[MultiApps]
  [viz]
    type = TransientMultiApp
    input_files = '3d_subchannel.i'
    execute_on = 'final'
  []

  # [wrapper_interwrapper]
  #   type = TransientMultiApp
  #   input_files = '7_assemblies_plus_subchannel.i'
  #   execute_on = 'TIMESTEP_END'
  # []

  [pronghorn]
    app_type = PronghornApp
    type = FullSolveMultiApp
    input_files = '7_assemblies_overlap_subchannel.i'
    execute_on = 'timestep_end'
    positions = '0 0 0'
    output_in_position = true
    bounding_box_padding = '0 0 0.1'
  []
[]

[Transfers]
  [subchannel_transfer]
    type = MultiAppDetailedSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu S'
  []
  [pin_transfer]
    type = MultiAppDetailedPinSolutionTransfer
    to_multi_app = viz
    variable = 'Tpin q_prime'
  []

  [pressure_drop_transfer] # Send pressure drop to pronghorn
    type = MultiAppPostprocessorTransfer
    to_multi_app =  pronghorn
    from_postprocessor = total_pressure_drop
    to_postprocessor = report_pressure_drop
    execute_on = 'timestep_end'
  []

  [mass_flux_tranfer] # Recover mass_flux at the inlet from pronghorn
    type = MultiAppPostprocessorTransfer
    from_multi_app = pronghorn
    from_postprocessor = inlet_mass_flux
    to_postprocessor = report_mass_flux_inlet
    execute_on = 'timestep_end'
  []

  [outlet_pressure_tranfer] # Recover pressure at the outlet from pronghorn
    type = MultiAppPostprocessorTransfer
    from_multi_app = pronghorn
    from_postprocessor = outlet_average_pressure
    to_postprocessor = report_pressure_outlet
    execute_on = 'timestep_end'
  []

  # [duct_temperature_transfer] # Send duct temperature to heat conduction
  #   type = MultiAppInterpolationTransfer
  #   to_multi_app = wrapper_interwrapper
  #   source_variable = Tduct
  #   variable = duct_surface_temperature
  # []

  # [q_prime_duct] # Recover q_prime from heat conduction solve
  #   type = MultiAppInterpolationTransfer
  #   from_multi_app = wrapper_interwrapper
  #   source_variable = q_prime_duct
  #   variable = q_prime_duct
  # []
[]
