num_cells = 15
T_in = 359.15
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 1e+6 * 17.00 / 3600.}'
P_out = 4.923e6 # Pa
length = 0.5

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadSubChannelMeshGenerator
    nx = 3
    ny = 3
    n_cells = ${num_cells}
    pitch = 0.25
    pin_diameter = 0.125
    gap = 0.1
    unheated_length_entry = 0.5
    heated_length = 0.5
    unheated_length_exit = 0.5
  []

  [fuel_pins]
    type = SCMQuadPinMeshGenerator
    input = sub_channel
    nx = 3
    ny = 3
    n_cells = ${num_cells}
    pitch = 0.25
    unheated_length_entry = 0.5
    heated_length = 0.5
    unheated_length_exit = 0.5
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

[AuxVariables]
  [q_prime_aux]
    block = fuel_pins
  []
  [q_prime]
    block = fuel_pins
  []
  [mdot]
    block = sub_channel
  []
  [SumWij]
    block = sub_channel
  []
  [P]
    block = sub_channel
  []
  [DP]
    block = sub_channel
  []
  [h]
    block = sub_channel
  []
  [T]
    block = sub_channel
  []
  [Tpin]
    block = fuel_pins
  []
  [rho]
    block = sub_channel
  []
  [mu]
    block = sub_channel
  []
  [S]
    block = sub_channel
  []
  [w_perim]
    block = sub_channel
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[SubChannel]
  type = QuadSubChannel1PhaseProblem
  fp = water
  n_blocks = 1
  beta = 0.08
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
  verbose_subchannel = true
  implicit = true
  segregated = false
[]

[AuxKernels]
  [q_prime_AUX]
    type = SCMQuadPowerAux
    variable = q_prime_aux
    power = 1e6 # W
    filename = "power_profile.txt" #type in name of file that describes radial power profile
    execute_on = 'initial'
    axial_heat_rate = axial_heat_rate
  []
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
[]

[ICs]
  [q_prime_IC]
    type = SCMQuadPowerIC
    variable = q_prime
    power = 1e6 # W
    filename = "power_profile.txt" #type in name of file that describes radial power profile
    axial_heat_rate = axial_heat_rate
  []

  [S_IC]
    type = SCMQuadFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = SCMQuadWettedPerimIC
    variable = w_perim
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = 0.00950
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
    fp = water
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = water
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = water
  []

  [mdot_ic]
    type = ConstantIC
    variable = mdot
    value = 0.0
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
    start_point = '0.125 0.125 0'
    end_point = '0.125 0.125 1.5'
    num_points = ${fparse num_cells + 1}
  []
[]

[Outputs]
  csv = true
[]
