T_in = 297.039 # K
P_out = 101325 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = QuadSubChannelMeshGenerator
    nx = 7
    ny = 3
    n_cells = 60
    n_blocks = 1
    pitch = 0.014605
    rod_diameter = 0.012065
    gap = 0.0015875
    unheated_length_entry = 0.3048
    heated_length = 1.2192
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
  CT = 2.6
  P_tol = 1e-6
  T_tol = 1e-6
  compute_density = true
  compute_viscosity = true
  compute_power = false
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
    power = 0.0  # W
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
    value = 0.012101767481985
  []
[]

[Functions]
  [mass_flux_in]
    type = PiecewiseLinear
    xy_data = '
      5.0 122.2645
      10.0 42.7926'
  []
[]

[Controls]
  [mass_flux_ctrl]
    type = RealFunctionControl
    parameter = 'AuxKernels/mdot_in_bc/mass_flux'
    function = 'mass_flux_in'
    execute_on = 'initial timestep_begin'
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
    mass_flux = 0.0
    execute_on = 'timestep_begin'
  []
[]

[Postprocessors]
  [mdot]
    type = QuadSubChannelPointValue
    variable = mdot
    ix = 3
    iy = 1
    execute_on = 'initial timestep_end'
    height = 0.4953
  []

  [mdot2]
    type = QuadSubChannelPointValue
    variable = mdot
    ix = 3
    iy = 1
    execute_on = 'initial timestep_end'
    height = 0.0
  []
[]

[Outputs]
  exodus = true
  csv = true
[]

[Executioner]
  type = Transient
  nl_rel_tol = 0.9
  l_tol = 0.9
  start_time = 0.0
  end_time = 10
  dt = 1.0
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  [viz]
    type = TransientMultiApp
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
