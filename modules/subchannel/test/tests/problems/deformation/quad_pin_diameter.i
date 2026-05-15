T_in = 359.15
P_out = 4.923e6
mass_flux_in = '${fparse 1e+6 * 17.00 / 3600.}'

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadSubChannelMeshGenerator
    nx = 3
    ny = 3
    n_cells = 5
    pitch = 0.014605
    pin_diameter = 0.012
    side_gap = 0.0015875
    heated_length = 0.5
  []
  [fuel_pins]
    type = SCMQuadPinMeshGenerator
    input = sub_channel
    nx = 3
    ny = 3
    n_cells = 5
    pitch = 0.014605
    heated_length = 0.5
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
  compute_power = false
  P_out = ${P_out}
  implicit = true
  segregated = false
  friction_closure = 'MATRA'
  pin_HTC_closure = 'Dittus-Boelter'
  full_output = true
[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []
  [Dittus-Boelter]
    type = SCMHTCDittusBoelter
  []
[]

[ICs]
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
    value = 0.012065
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
  [S_deformed]
    type = SubChannelPointValue
    variable = S
    index = 4
    height = 0.25
    execute_on = 'timestep_end'
  []
  [w_perim_deformed]
    type = SubChannelPointValue
    variable = w_perim
    index = 4
    height = 0.25
    execute_on = 'timestep_end'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  console = true
  csv = true
[]
