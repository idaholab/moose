T_in = 473.15 # K
mass_flux_in = 3500 # kg /sec m2
P_out = 155e+5 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 2
    ny = 1
    n_cells = 100
    pitch = 0.0126
    rod_diameter = 0.00950
    gap = 0.00095 # the half gap between sub-channel assemblies
    heated_length = 10.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[Functions]
  [S_fn]
    type = ParsedFunction
    value = if(x>0.0,0.002,0.001)
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
  CT = 0.0
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
  # [Temp_Out_MATRIX]
  #   type = QuadSubChannelNormalSliceValues
  #   variable = T
  #   execute_on = final
  #   file_base = "Temp_Out.txt"
  #   height = 10.0
  # []
  # [mdot_Out_MATRIX]
  #   type = QuadSubChannelNormalSliceValues
  #   variable = mdot
  #   execute_on = final
  #   file_base = "mdot_Out.txt"
  #   height = 10.0
  # []
  # [mdot_In_MATRIX]
  #   type = QuadSubChannelNormalSliceValues
  #   variable = mdot
  #   execute_on = final
  #   file_base = "mdot_In.txt"
  #   height = 0.0
  # []
[]

[Executioner]
  type = Steady
[]
