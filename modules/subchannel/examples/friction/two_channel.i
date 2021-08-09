T_in = 473.15 # K
mass_flux_in = 3500 # kg /sec m2
P_out = 155e+5 # Pa

[Mesh]
  type = QuadSubChannelMesh
  nx = 1
  ny = 2
  n_cells = 100
  n_blocks = 1
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095 # the half gap between sub-channel assemblies
  heated_length = 10.0
  spacer_z = '0.0'
  spacer_k = '0.0'
[]

[Functions]
  [S_fn]
    type = ParsedFunction
    value = if(y>0.0,0.002,0.001)
  []
[]

[AuxVariables]
  [mdot]
  []
  [SumWij]
  []
  [P]
  []
  [DP]
  []
  [h]
  []
  [T]
  []
  [rho]
  []
  [mu]
  []
  [S]
  []
  [w_perim]
  []
  [q_prime]
  []
[]

[Modules]
  [FluidProperties]
    [water]
      type = Water97FluidProperties
    []
  []
[]

[Problem]
  type = LiquidWaterSubChannel1PhaseProblem
  fp = water
  beta = 0.006
  CT = 0.0
  P_tol = 1e-6
  T_tol = 1e-6
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
[]

[ICs]
  [S_ic]
    type = FunctionIC
    variable = S
    function = S_fn
  []

  [w_perim_IC]
    type = ConstantIC
    variable = w_perim
    value = 0.34188034
  []

  [q_prime_IC]
    type = ConstantIC
    variable = q_prime
    value = 0.0
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
  [Temp_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_Out.txt"
    height = 10.0
  []
  [mdot_Out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_Out.txt"
    height = 10.0
  []
  [mdot_In_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_In.txt"
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
  [pretty_mesh]
    type = FullSolveMultiApp
    input_files = "pretty_mesh.i"
    execute_on = "timestep_end"
  []
[]

[Transfers]
  [xfer_mdot]
    type = MultiAppNearestNodeTransfer
    multi_app = pretty_mesh
    direction = to_multiapp
    source_variable = mdot
    variable = mdot
  []
  [xfer_SumWij]
    type = MultiAppNearestNodeTransfer
    multi_app = pretty_mesh
    direction = to_multiapp
    source_variable = SumWij
    variable = SumWij
  []
  [xfer_P]
    type = MultiAppNearestNodeTransfer
    multi_app = pretty_mesh
    direction = to_multiapp
    source_variable = P
    variable = P
  []
  [xfer_DP]
    type = MultiAppNearestNodeTransfer
    multi_app = pretty_mesh
    direction = to_multiapp
    source_variable = DP
    variable = DP
  []
  [xfer_h]
    type = MultiAppNearestNodeTransfer
    multi_app = pretty_mesh
    direction = to_multiapp
    source_variable = h
    variable = h
  []
  [xfer_T]
    type = MultiAppNearestNodeTransfer
    multi_app = pretty_mesh
    direction = to_multiapp
    source_variable = T
    variable = T
  []
  [xfer_rho]
    type = MultiAppNearestNodeTransfer
    multi_app = pretty_mesh
    direction = to_multiapp
    source_variable = rho
    variable = rho
  []
  [xfer_mu]
    type = MultiAppNearestNodeTransfer
    multi_app = pretty_mesh
    direction = to_multiapp
    source_variable = mu
    variable = mu
  []
  [xfer_q_prime]
    type = MultiAppNearestNodeTransfer
    multi_app = pretty_mesh
    direction = to_multiapp
    source_variable = q_prime
    variable = q_prime
  []
  [xfer_S]
    type = MultiAppNearestNodeTransfer
    multi_app = pretty_mesh
    direction = to_multiapp
    source_variable = S
    variable = S
  []
[]
