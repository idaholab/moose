T_in = 297.039 # K
P_out = 101325 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadSubChannelMeshGenerator
    nx = 7
    ny = 3
    n_cells = 60
    pitch = 0.014605
    pin_diameter = 0.012065
    side_gap = 0.0015875
    unheated_length_entry = 0.3048
    heated_length = 1.2192
    spacer_z = '0.0'
    spacer_k = '0.0'
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
  CT = 2.6
  P_tol = 1e-6
  T_tol = 1e-6
  compute_density = true
  compute_viscosity = true
  compute_power = false
  P_out = ${P_out}
  friction_closure = 'Pang'
[]

[SCMClosures]
  [Pang]
    type = SCMFrictionBoPang
  []
[]

[ICs]
  [S_IC]
    type = SCMQuadFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = SCMQuadWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = SCMQuadPowerIC
    variable = q_prime
    power = 0.0 # W
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
  [mass_flux]
    type = PiecewiseLinear
    xy_data = '
      5.0 122.2645
      10.0 42.7926'
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
    mass_flux = mass_flux_in
    execute_on = 'timestep_begin'
  []
[]

[Postprocessors]
  [mass_flux_in]
    type = FunctionValuePostprocessor
    function = mass_flux
    execute_on = 'initial timestep_end'
  []
  [mdot]
    type = SubChannelPointValue
    variable = mdot
    index = 4
    execute_on = 'initial timestep_end'
    height = 0.4953
  []

  [mdot2]
    type = SubChannelPointValue
    variable = mdot
    index = 4
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
    type = SCMSolutionTransfer
    to_multi_app = viz
    variable = 'mdot SumWij P DP h T rho mu q_prime S'
  []
[]
