mass_flux_in = 3500 # kg /sec m2
T_in = 297.039 # K
P_out = 101325 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 3
    ny = 3
    n_cells = 100
    n_blocks = 1
    pitch = 0.0126
    rod_diameter = 0.00950
    gap = 0.00095
    unheated_length_entry = 2.5
    heated_length = 5.0
    unheated_length_exit = 2.5
    spacer_z = '0.0'
    spacer_k = '0.0'
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
  type = LiquidWaterSubChannel1PhaseProblem
  fp = water
  beta = 0.006
  CT = 2.0
  P_tol = 1e-6
  T_tol = 1e-6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
[]

[ICs]
  [S_IC]
    type = QuadFlowAreaIC
    variable = S
  []

  [T_ic]
    type = ConstantIC
    variable = T
    value = ${T_in}
  []

  [w_perim_IC]
    type = QuadWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = QuadPowerIC
    variable = q_prime
    power = 10000.0  # W
    filename = "power_profile.txt"
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
  [h_3]
    type = QuadSubChannelNormalSliceValues
    variable = h
    execute_on = final
    file_base = "h_3.txt"
    height = 10.0
  []
  [h_2]
    type = QuadSubChannelNormalSliceValues
    variable = h
    execute_on = final
    file_base = "h_2.txt"
    height = 7.5
  []
  [h_1]
    type = QuadSubChannelNormalSliceValues
    variable = h
    execute_on = final
    file_base = "h_1.txt"
    height = 2.5
  []
  [h_0]
    type = QuadSubChannelNormalSliceValues
    variable = h
    execute_on = final
    file_base = "h_0.txt"
    height = 0.0
  []
  [mdot_3]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_3.txt"
    height = 10.0
  []
  [mdot_2]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_2.txt"
    height = 7.5
  []
  [mdot_1]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_1.txt"
    height = 2.5
  []
  [mdot_0]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_0.txt"
    height = 0.0
  []
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
    input_files = "3d.i"
    execute_on = "timestep_end"
  []
[]

[Transfers]
  [xfer]
    type = MultiAppDetailedSolutionTransfer
    multi_app = viz
    direction = to_multiapp
    variable = 'mdot SumWij P DP h T rho mu q_prime S'
  []
[]
