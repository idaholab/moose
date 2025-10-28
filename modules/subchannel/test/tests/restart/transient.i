T_in = 359.15
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 1e+6 * 17.00 / 3600.}'
P_out = 4.923e6 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadSubChannelMeshGenerator
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

[UserObjects]
  [steady_sln]
    type = SolutionUserObject
    mesh = steady_out.e
    timestep = LATEST
    system_variables = 'mdot SumWij P DP h T Tpin rho mu S w_perim q_prime'
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
  beta = 0.006
  CT = 1.8
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
  restart_file_base = steady_out_cp/LATEST
  skip_additional_restart_data = true
  allow_initial_conditions_with_restart = true
  friction_closure = 'Pang'
[]

[SCMClosures]
  [Pang]
    type = SCMFrictionBoPang
  []
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

  [mu_ic_fn]
    type = SolutionFunction
    solution = steady_sln
    from_variable = mu
  []
[]

[ICs]
  [S_ic]
    type = SCMQuadFlowAreaIC
    variable = S
  []

  [w_perim_ic]
    type = SCMQuadWettedPerimIC
    variable = w_perim
  []

  [q_prime_ic]
    type = SCMQuadPowerIC
    variable = q_prime
    power = 1e6
    filename = "power_profile.txt"
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

  [viscosity_ic]
    type = FunctionIC
    variable = mu
    function = mu_ic_fn
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
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  exodus = true
  hide = ff
[]

[Executioner]
  type = Transient
  start_time = 0.0
  end_time = 0.2
  dt = 0.1
[]
