T_in = 360.0
mass_flux_in = ${fparse 1e+4 * 17.0 / 3600.}
P_out = 4.923e6 # Pa


[QuadInterWrapperMesh]
    [sub_channel]
      type = QuadInterWrapperMeshGenerator
      nx = 15
      ny = 15
      n_cells = 50
      assembly_pitch = 0.2
      assembly_side_x = 0.18
      assembly_side_y = 0.18
      side_bypass = 0.001
      heated_length = 3.0
    []
[]


[AuxVariables]
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
  [q_prime]
    block = sub_channel
  []
[]


[Modules]
  [FluidProperties]
    [water]
      type = Water97FluidProperties
    []
  []
[]


[SubChannel]
  type = LiquidWaterInterWrapper1PhaseProblem
  fp = water
  n_blocks = 1
  beta = 0.08
  CT = 2.6
  P_tol = 1e-6
  T_tol = 1e-6
  compute_density = true
  compute_viscosity = true
  compute_power = false
  P_out = ${P_out}
  implicit = false
  segregated = true
  staggered_pressure = false
  monolithic_thermal = false
  discretization = "central_difference"
[]


[ICs]
  [S_IC]
    type = QuadInterWrapperFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = QuadInterWrapperWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = QuadInterWrapperPowerIC
    variable = q_prime
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
    type = MassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]


[Outputs]
  exodus = true
  checkpoint = false
[]


[Executioner]
  type = Steady
  nl_rel_tol = 0.9
  l_tol = 0.9
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  [viz]
    type = FullSolveMultiApp
    input_files = "inter_wrapper_3d.i"
    execute_on = "timestep_end"
  []
[]

[Transfers]
  [xfer]
    type = MultiAppInterWrapperSolutionTransfer
    multi_app = viz
    direction = to_multiapp
    variable = 'mdot SumWij P DP h T rho mu q_prime S'
  []
[]
