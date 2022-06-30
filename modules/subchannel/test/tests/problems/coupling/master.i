T_in = 297.039 # K
P_out = 101325 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 2
    ny = 2
    n_cells = 10
    pitch = 0.014605
    rod_diameter = 0.012065
    gap = 0.0015875
    heated_length = 1.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []

  [fuel_pins]
    type = QuadPinMeshGenerator
    input = sub_channel
    nx = 2
    ny = 2
    n_cells = 10
    pitch = 0.014605
    heated_length = 1.0
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
  [Tpin]
    block = fuel_pins
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
    block = fuel_pins
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
  n_blocks = 1
  fp = water
  beta = 0.006
  CT = 2.6
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
     power = 1000  # W
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

[UserObjects]
  [Tpin_avg_uo]
    type = NearestPointLayeredAverage
    direction = z
    num_layers = 10
    variable = Tpin
    block = fuel_pins
    points = '0 0 0'
    execute_on = timestep_end
  []
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 30
  fixed_point_min_its = 2
  fixed_point_rel_tol = 1e-6
[]

################################################################################
# A multiapp that transfers data to BISON simulations
################################################################################

[MultiApps] # I have as many multiapps as pins
  [SLAVE]
    type = FullSolveMultiApp
    input_files = slave.i # seperate file for multiapps due to radial power profile
    execute_on = 'timestep_end'
    positions = '0   0   0 '
    bounding_box_padding = '0 0 0.01'
  []
[]

[Transfers]
  [Tpin] # send pin surface temperature to bison,
    type = MultiAppUserObjectTransfer2
    to_multi_app = SLAVE
    variable = Pin_surface_temperature
    user_object = Tpin_avg_uo
  []
#
  [from_SLAVE] # send heat flux from BISON to subchannel
    type = MultiAppUserObjectTransfer2
    from_multi_app = SLAVE
    variable = q_prime
    user_object = q_prime_uo
    execute_on = 'timestep_end'
  []
[]
