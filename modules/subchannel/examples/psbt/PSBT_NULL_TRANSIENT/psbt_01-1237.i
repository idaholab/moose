T_in = 359.15
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = ${fparse 1e+6 * 17.00 / 3600.}
P_out = 4.923e6 # Pa

[Mesh]
  type = BetterQuadSubChannelMesh
  nx = 6
  ny = 6
  n_cells = 50
  n_blocks = 1
  pitch = 0.0126
  rod_diameter = 0.00950
  gap = 0.00095 # the half gap between sub-channel assemblies
  heated_length = 3.658
  spacer_z = '0 0.229 0.457 0.686 0.914 1.143 1.372 1.600 1.829 2.057 2.286 2.515 2.743 2.972 3.200 3.429'
  spacer_k = '0.7 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4 1.0 0.4'
[]

[UserObjects]
  [steady_sln]
    type = SolutionUserObject
    mesh = psbt_01-1237_out_SS.e
    timestep = LATEST
    system_variables = 'mdot SumWij P DP h T rho Mu S w_perim q_prime'
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
  [Mu]
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
  type = BetterSubChannel1PhaseProblemBase
  fp = water
  beta = 0.006
  CT = 1.8
  enforce_uniform_pressure = false
  Density = true
  Viscosity = true
  Power = true
  P_out = ${P_out}
[]

[Functions]
  [mdot_ic_fn]
    type = SolutionFunction
    solution = steady_sln
    from_variable = mdot
  []

  [P_ic_fn]
    type = SolutionFunction
    solution = steady_sln
    from_variable = P
  []

  [DP_ic_fn]
    type = SolutionFunction
    solution = steady_sln
    from_variable = DP
  []

  [h_ic_fn]
    type = SolutionFunction
    solution = steady_sln
    from_variable = h
  []

  [T_ic_fn]
    type = SolutionFunction
    solution = steady_sln
    from_variable = T
  []

  [rho_ic_fn]
    type = SolutionFunction
    solution = steady_sln
    from_variable = rho
  []

  [Mu_ic_fn]
    type = SolutionFunction
    solution = steady_sln
    from_variable = Mu
  []
[]


[ICs]
  [S_IC]
    type = BetterQuadFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = BetterQuadWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = BetterQuadPowerIC
    variable = q_prime
    power = 3.44e6 # W
    filename = "power_profile.txt" #type in name of file that describes power profile
  []

  [T_ic]
  type = FunctionIC
  variable = T
  function = T_ic_fn
  []

  [P_ic]
  type = FunctionIC
  variable = P
  function = P_ic_fn
  []

  [DP_ic]
    type = FunctionIC
    variable = DP
    function = DP_ic_fn
  []

  [Viscosity_ic]
    type = FunctionIC
    variable = Mu
    function = Mu_ic_fn
  []

  [rho_ic]
    type = FunctionIC
    variable = rho
    function = rho_ic_fn
  []

  [h_ic]
    type = FunctionIC
    variable = h
    function = h_ic_fn
  []

  [mdot_ic]
    type = FunctionIC
    variable = mdot
    function = mdot_ic_fn
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
    type = BetterNormalSliceValues
    variable = T
    execute_on = TIMESTEP_END
    file_base = "Temp_Out.txt"
    height = 3.658
  []
  [mdot_Out_MATRIX]
    type = BetterNormalSliceValues
    variable = mdot
    execute_on = TIMESTEP_END
    file_base = "mdot_Out.txt"
    height = 3.658
  []
  [mdot_In_MATRIX]
    type = BetterNormalSliceValues
    variable = mdot
    execute_on = TIMESTEP_END
    file_base = "mdot_In.txt"
    height = 0.0
  []
[]

[Executioner]
  type = Transient
  nl_rel_tol = 0.9
  l_tol = 0.9
  start_time = 0.0
  end_time = 0.2
  dt = 0.1
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
  [xfer_Mu]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = Mu
    variable = Mu
  []
  [xfer_q_prime]
    type = MultiAppNearestNodeTransfer
    multi_app = prettyMesh
    direction = to_multiapp
    source_variable = q_prime
    variable = q_prime
  []
[]
