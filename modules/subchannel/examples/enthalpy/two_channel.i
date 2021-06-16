
mass_flux_in = 3500 # kg /sec m2
P_out = 155e+5 # Pa

[Mesh]
  type = BetterQuadSubChannelMesh
  nx = 2
  ny = 1
  n_cells = 100
  n_blocks = 1
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095 # the half gap between sub-channel assemblies
  heated_length = 10.0
  spacer_z = '0 0.229 0.457 0.686 0.914 1.143 1.372 1.600 1.829 2.057 2.286 2.515 2.743 2.972 3.200 3.429'
  spacer_k = '0.7 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4'
[]

[Functions]
  [T_fn]
    type = ParsedFunction
    value = if(x>0.0,483.10,473.10)
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
  type = BetterSubChannel1PhaseProblem
  fp = water
  beta = 0.006
  CT = 1.8
  enforce_uniform_pressure = false
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
[]

[ICs]
  [S_ic]
    type = ConstantIC
    variable = S
    value = 8.78778158e-05
  []

  [T_ic]
    type = FunctionIC
    variable = T
    function = T_fn
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
    type = BetterNormalSliceValues
    variable = T
    execute_on = final
    file_base = "Temp_Out.txt"
    height = 10.0
  []
  [mdot_Out_MATRIX]
    type = BetterNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_Out.txt"
    height = 10.0
  []
  [mdot_In_MATRIX]
    type = BetterNormalSliceValues
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
  [prettyMesh]
    type = FullSolveMultiApp
    input_files = "prettyMesh.i"
    execute_on = "timestep_end"
  []
[]

[Transfers]
  [xfer_mdot]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = mdot
    variable = mdot
  []
  [xfer_SumWij]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = SumWij
    variable = SumWij
  []
  [xfer_P]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = P
    variable = P
  []
  [xfer_DP]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = DP
    variable = DP
  []
  [xfer_h]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = h
    variable = h
  []
  [xfer_T]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = T
    variable = T
  []
  [xfer_rho]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = rho
    variable = rho
  []
  [xfer_mu]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = mu
    variable = mu
  []
  [xfer_q_prime]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = q_prime
    variable = q_prime
  []
[]
