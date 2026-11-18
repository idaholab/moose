T_in = 359.15
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 1e+6 * 17.00 / 3600.}'
P_out = 4.923e6 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadAssemblyMeshGenerator
    nx = 3
    ny = 3
    n_cells = 10
    pitch = 0.0126
    pin_diameter = 0.00950
    side_gap = 0.00095
    heated_length = 1
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[AuxVariables]
  [Dpin]
    block = fuel_pins
  []
  [Tduct]
    block = subchannel
  []
  [Tpin]
    block = fuel_pins
  []
  [duct_heat_flux]
    block = subchannel
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
  full_output = true
  mixing_closure ='constant_beta'
  pin_HTC_closure = 'Dittus-Boelter'

[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []
  [constant_beta]
    type = SCMMixingConstantBeta
    beta = 0.006
    CT = 1.8
  []
  [Dittus-Boelter]
    type = SCMHTCDittusBoelter
  []
[]

[ICs]


  [q_prime_ic]
    type = SCMQuadPowerIC
    variable = q_prime
    power = 1e6
    filename = "power_profile.txt"
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

  [Dpin_ic]
    type = ConstantIC
    variable = Dpin
    value = 0.00950
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
  [T_out]
    type = SCMPlanarMean
    variable = T
    height = 1
    execute_on = 'timestep_end'
  []
  [DP]
    type = SubChannelDelta
    variable = P
    execute_on = 'timestep_end'
  []
  # Center subchannel of the 3x3 array
  [T_center_mid]
    type = SubChannelPointValue
    variable = T
    index = 4
    height = 0.5
    execute_on = 'timestep_end'
  []
  # Corner subchannel of the 3x3 array
  [mdot_corner_out]
    type = SubChannelPointValue
    variable = mdot
    index = 0
    height = 1
    execute_on = 'timestep_end'
  []
[]

[VectorPostprocessors]
  # Every nodal value of the subchannel variables, so a variable that is not
  # restored from the checkpoint shows up in the time = 0 transient output
  [fluid]
    type = NodalValueSampler
    block = subchannel
    variable = 'mdot SumWij P DP h T rho mu S w_perim displacement ff HTC'
    sort_by = id
    execute_on = 'timestep_end'
  []
  [pins]
    type = NodalValueSampler
    block = fuel_pins
    variable = 'q_prime Tpin Dpin'
    sort_by = id
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
  checkpoint = true
  hide = 'ff'
[]

[Executioner]
  type = Steady
[]
