T_in = 588.5
flow_area = 0.0004980799633447909 #m2
mass_flux_in = '${fparse 55*3.78541/10/60/flow_area}'
P_out = 2.0e5 # Pa
length = 0.5
num_cells = 40
[TriSubChannelMesh]
  [subchannel]
    type = SCMTriSubChannelMeshGenerator
    nrings = 3
    n_cells = ${num_cells}
    flat_to_flat = 3.41e-2
    heated_length = 0.5
    unheated_length_entry = 0.4
    unheated_length_exit = 0.1
    pin_diameter = 5.84e-3
    pitch = 7.26e-3
    dwire = 1.42e-3
    hwire = 0.3048
  []

  [fuel_pins]
    type = SCMTriPinMeshGenerator
    input = subchannel
    nrings = 3
    n_cells = ${num_cells}
    heated_length = 0.5
    unheated_length_entry = 0.4
    unheated_length_exit = 0.1
    pitch = 7.26e-3
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
  [w_perim]
    block = subchannel
  []
  [mu]
    block = subchannel
  []
  [displacement]
    block = subchannel
  []
  [q_prime]
    block = fuel_pins
  []
  [q_prime_aux]
    block = fuel_pins
  []
  [Tpin]
    block = fuel_pins
  []
  [Dpin]
    block = fuel_pins
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    value = '(pi/2)*sin(pi*z/L)'
    vars = 'L'
    vals = '${length}'
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
  P_out = 2.0e5
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  implicit = true
  segregated = false
  verbose_subchannel = true
  interpolation_scheme = upwind
[]

[ICs]
  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = 5.84e-3
  []

  [S_IC]
    type = SCMTriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = SCMTriWettedPerimIC
    variable = w_perim
  []

  [q_prime_ic]
    type = SCMTriPowerIC
    variable = q_prime
    power = 20000 # W
    filename = "pin_power_profile.txt"
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
  [T_in_bc]
    type = ConstantAux
    variable = T
    boundary = inlet
    value = ${T_in}
    execute_on = 'timestep_begin'
  []
  [mdot_in_bc]
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
  [q_prime_AUX]
    type = SCMTriPowerAux
    variable = q_prime_aux
    power = 20000 # W
    filename = "pin_power_profile.txt" #type in name of file that describes radial power profile
    axial_heat_rate = axial_heat_rate
    execute_on = 'initial'
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [Total_power_IC_defaultPP]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime
    block = fuel_pins
  []
  [Total_power_Aux_defaultPP]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime_aux
    block = fuel_pins
  []
  [Total_power_SCMPowerPostprocessor]
    type = SCMPowerPostprocessor
  []
[]

[VectorPostprocessors]
  [line_check]
    type = LineValueSampler
    variable = 'q_prime q_prime_aux'
    execute_on = 'TIMESTEP_END'
    sort_by = 'z'
    start_point = '0 0 0'
    end_point = '0 0 1.0'
    num_points = ${fparse num_cells + 1}
  []
[]

[Outputs]
  csv = true
[]
