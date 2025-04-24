# Following Advanced Burner Test Reactor Preconceptual Design Report
# Vailable at: https://www.ne.anl.gov/eda/ABTR_1cv2_ws.pdf
###################################################
# Thermal-hydraulics parameters
###################################################
T_in = 866.0
P_out = 253727.1   # Pa
reactor_power = 671337.24 #WTh
mass_flow = '${fparse 6.15}' # kg/(s)

###################################################
# Geometric parameters
###################################################

# units are cm - do not forget to convert to meter
scale_factor = 0.01
fuel_pin_pitch = '${fparse 1.4478*scale_factor}'
fuel_pin_diameter = '${fparse 1.4268*scale_factor}'
wire_z_spacing = '${fparse 0*scale_factor}'
wire_diameter = '${fparse 0*scale_factor}'
n_rings = 8
length_heated_fuel = '${fparse 35.56*scale_factor}'
duct_inside = '${fparse 11.43*2*scale_factor}'

entry1 = '${fparse 0/100}'
entry2 = '${fparse 0/100}'
entry3 = '${fparse 0/100}'
entry_length = '${fparse entry1 + entry2 + entry3}'
exit1 = '${fparse 0/100}'
exit2 = '${fparse 0/100}'
exit3 = '${fparse 0/100}'
exit_length = '${fparse exit1 + exit2 + exit3}'
###################################################

[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = '${fparse n_rings}'
    n_cells = 10
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse entry_length}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse exit_length}'
    pin_diameter = '${fparse fuel_pin_diameter}'
    pitch = '${fparse fuel_pin_pitch}'
    dwire = '${fparse wire_diameter}'
    hwire = '${fparse wire_z_spacing}'
    spacer_z = '0'
    spacer_k = '0'
  []

  [fuel_pins]
    type = SCMTriPinMeshGenerator
    input = subchannel
    nrings = '${fparse n_rings}'
    n_cells = 10
    unheated_length_entry = '${fparse entry_length}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse exit_length}'
    pitch = '${fparse fuel_pin_pitch}'
  []

  [duct]
    type = SCMTriDuctMeshGenerator
    input = fuel_pins
    nrings = '${fparse n_rings}'
    n_cells = 10
    flat_to_flat = '${fparse duct_inside}'
    unheated_length_entry = '${fparse entry_length}'
    heated_length = '${fparse length_heated_fuel}'
    unheated_length_exit = '${fparse exit_length}'
    pitch = '${fparse fuel_pin_pitch}'
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
  [Dpin]
    block = fuel_pins
  []
  [rho]
    block = subchannel
  []
  [S]
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
  [displacement]
    block = subchannel
    initial_condition = 0
  []
[]

[FluidProperties]
  [sodium]
    type = SimpleFluidProperties
    molar_mass = 0.0355
    cp = 873.0
    cv = 873.0
    specific_entropy = 1055
    viscosity = 0.0001582
    thermal_conductivity = 25.9
    thermal_expansion = 2.77e-4
  []
[]

[Problem]
  type = TriSubChannel1PhaseProblem
  fp = sodium
  n_blocks = 1
  P_out = ${P_out}
  CT = 1.0
  P_tol = 1.0e-2
  T_tol = 1.0e-2

  # Solver settings
  implicit = true
  segregated = false
  staggered_pressure = false
  monolithic_thermal = false

  # Output
  verbose_multiapps = true
  verbose_subchannel = true
  compute_density = false
  compute_viscosity = false
  compute_power = false
[]


[ICs]
  [S_IC]
    type = SCMTriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = SCMTriWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = SCMTriPowerIC
    variable = q_prime
    power = ${reactor_power} # W
    filename = 'pin_p.txt'
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = ${fuel_pin_diameter}
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

  [T_duct_ic]
    type = ConstantIC
    variable = Tduct
    value = ${T_in}
  []
  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
  []
[]

[AuxKernels]
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
    block = subchannel
  []
  [mdot_in_bc]
    type = SCMFlatMassFlowRateAux
    variable = mdot
    boundary = inlet
    mass_flow = ${mass_flow}
    execute_on = 'timestep_begin'
    block = subchannel
  []
[]

[Executioner]
  type = Steady
[]

[VectorPostprocessors]
  [sub]
    type = LineValueSampler
    start_point = '0 -0.00835888 ${entry_length}'
    end_point = '0 -0.00835888 ${fparse entry_length + length_heated_fuel}'
    num_points = 10
    variable = 'h rho P'
    sort_by = 'z'
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
[]
