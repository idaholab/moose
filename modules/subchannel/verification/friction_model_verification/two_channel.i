T_in = 473.15 # K
mass_flux_in = 3500 # kg /sec m2
P_out = 155e+5 # Pa

[QuadSubChannelMesh]
  [sub_channel]
    type = SCMQuadSubChannelMeshGenerator
    nx = 1
    ny = 2
    n_cells = 100
    pitch = 0.0126
    pin_diameter = 0.00950
    side_gap = 0.00095
    heated_length = 10.0
    spacer_z = '0.0'
    spacer_k = '0.0'
  []
[]

[Functions]
  [S_fn]
    type = ParsedFunction
    expression = if(y>0.0,0.002,0.001)
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
  P_tol = 1e-6
  T_tol = 1e-6
  implicit = false
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_out = ${P_out}
  friction_closure = 'MATRA'
[]

[SCMClosures]
  [MATRA]
    type = SCMFrictionMATRA
  []
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
    type = SCMMassFlowRateAux
    variable = mdot
    boundary = inlet
    area = S
    mass_flux = ${mass_flux_in}
    execute_on = 'timestep_begin'
  []
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [mdot_1]
    type = SubChannelPointValue
    variable = mdot
    index = 0
    execute_on = 'initial timestep_end'
    height = 10.0
  []
  [mdot_2]
    type = SubChannelPointValue
    variable = mdot
    index = 1
    execute_on = 'initial timestep_end'
    height = 10.0
  []
[]

[Executioner]
  type = Steady
[]

################################################################################
# A multiapp that projects data to a detailed mesh
################################################################################

[MultiApps]
  [viz]
    type = FullSolveMultiApp
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
