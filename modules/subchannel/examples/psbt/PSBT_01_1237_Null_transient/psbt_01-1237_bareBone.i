T_in = 359.15
# [1e+6 kg/m^2-hour] turns into kg/m^2-sec
mass_flux_in = ${fparse 1e+6 * 17.00 / 3600.}
P_out = 4.923e6 # Pa

[Mesh]
  type = QuadSubChannelMesh
  nx = 6
  ny = 6
  max_dz = 0.02
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
    mesh = psbt_01-1237_bareBone_out_SS.e
    timestep = LATEST
    system_variables = 'mdot P h T rho'
  []
[]

[AuxVariables]
  [mdot]
  []
  [SumWij]
  []
  [SumWijh]
  []
  [SumWijPrimeDhij]
  []
  [SumWijPrimeDUij]
  []
  [P]
  []
  [h]
  []
  [T]
  []
  [rho]
  []
  [S]
  []
  [Sij]
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
  type = SubChannel1PhaseProblem
  fp = water
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
[]

[ICs]
  [S_IC]
    type = QuadFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = WettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = PowerIC
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
  [P_out_bc]
    type = ConstantAux
    variable = P
    boundary = outlet
    value = ${P_out}
    execute_on = 'timestep_begin'
  []

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
  [mdot_Out_MATRIX]
    type = NormalSliceValues
    variable = mdot
    execute_on = TIMESTEP_END
    file_base = "mdot_Out_Transient.txt"
    height = 3.658
  []
  [mdot_in_MATRIX]
    type = NormalSliceValues
    variable = mdot
    execute_on = TIMESTEP_END
    file_base = "mdot_in_Transient.txt"
    height = 0.0
  []
  [Temp_Out_MATRIX]
    type = NormalSliceValues
    variable = T
    execute_on = TIMESTEP_END
    file_base = "Temp_Out_Transient.txt"
    height = 3.658
  []
  [Temp_in_MATRIX]
    type = NormalSliceValues
    variable = T
    execute_on = TIMESTEP_END
    file_base = "Temp_in_Transient.txt"
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
