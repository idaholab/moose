T_in = 359.15
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 1e+6 * 17.00 / 3600.}'
P_out = 4.923e6 # Pa
heated_length = 1.0

# Two pins at x = -pitch/2 and x = +pitch/2
[QuadSubChannelMesh]
  [subchannel]
    type = SCMQuadAssemblyMeshGenerator
    nx = 3
    ny = 2
    n_cells = 10
    pitch = 0.0126
    pin_diameter = 0.00950
    side_gap = 0.00095
    heated_length = ${heated_length}
  []
[]

[Functions]
  [axial_heat_rate]
    type = ParsedFunction
    expression = 'sin(pi*z/L)'
    symbol_names = 'L'
    symbol_values = '${heated_length}'
  []
  # Assembly power that doubles every time step
  [power_history]
    type = ParsedFunction
    expression = '1e4 * 2^(t/0.1)'
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
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
  friction_closure = 'MATRA'
  mixing_closure = 'constant_beta'
  pin_HTC_closure = 'Dittus-Boelter'
[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []
  [constant_beta]
    type = SCMMixingConstantBeta
    beta = 0.006
    CT = 2.6
  []
  [Dittus-Boelter]
    type = SCMHTCDittusBoelter
  []
[]

[ICs]
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

[AuxKernels]
  # Executes on the same flags as the power postprocessor, which therefore executes first, and
  # before the solve so the solver uses the power of the current time step
  [q_prime_AUX]
    type = SCMQuadPowerAux
    variable = q_prime
    power = power
    filename = 'two_pin_power_profile.txt'
    axial_heat_rate = axial_heat_rate
    execute_on = 'initial timestep_begin'
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

[Postprocessors]
  [power]
    type = FunctionValuePostprocessor
    function = power_history
    execute_on = 'initial timestep_begin'
  []
  [q_prime_power]
    type = ElementIntegralVariablePostprocessor
    variable = q_prime
    block = fuel_pins
    execute_on = 'initial timestep_end'
  []
  # Zero when q_prime carries the current value of the power postprocessor
  [power_mismatch]
    type = ParsedPostprocessor
    expression = 'q_prime_power - power'
    pp_names = 'q_prime_power power'
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 3
[]

[Outputs]
  csv = true
[]
