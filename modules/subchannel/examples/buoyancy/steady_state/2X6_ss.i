T_in = 297.039 # K
P_out = 101325 # Pa

[Mesh]
  type = QuadSubChannelMesh
  nx = 7
  ny = 3
  n_cells = 48
  n_blocks = 1
  pitch = 0.014605
  rod_diameter = 0.012065
  gap = 0.0015875
  heated_length = 1.2192
  spacer_z = '0.0 '
  spacer_k = '0.0 '
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

  [w_perim_IC]
    type = QuadWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = QuadPowerIC
    variable = q_prime
    power = 5460  # W
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
    mass_flux = 131.43435930715006
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  exodus = true
  [mdot_in_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_in.txt"
    height = 0.0
  []

  [rho_in_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = rho
    execute_on = final
    file_base = "rho_in.txt"
    height = 0.0
  []

  [mdot_out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_out.txt"
    height = 1.2192
  []

  [rho_out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = rho
    execute_on = final
    file_base = "rho_out.txt"
    height = 1.2192
  []

  [mdot_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = mdot
    execute_on = final
    file_base = "mdot_075.txt"
    height = 0.9144
  []

  [T_in_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "T_in.txt"
    height = 0.0
  []

  [T_out_MATRIX]
    type = QuadSubChannelNormalSliceValues
    variable = T
    execute_on = final
    file_base = "T_out.txt"
    height = 1.2192
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


