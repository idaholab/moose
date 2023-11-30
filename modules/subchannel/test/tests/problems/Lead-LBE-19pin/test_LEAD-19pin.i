T_in = 673.15
flow_area = 0.00128171 #m2
rho_in = 10453.21705
# [10 m^3/hour] turns into kg/m^2-sec
mass_flux_in = '${fparse 10*rho_in/3600/flow_area}'
P_out = 1.0e5 # Pa
[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = 3
    n_cells = 50
    flat_to_flat = 0.05319936
    heated_length = 0.87
    unheated_length_entry = 0.0
    unheated_length_exit = 0.402
    rod_diameter = 8.2e-3
    pitch = 0.01148
    dwire = 0.0
    hwire = 0.0
    spacer_z = '0.177 0.547 0.870'
    spacer_k = '1.1719 1.1719 1.1719'
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
  [S]
  []
  [w_perim]
  []
  [q_prime]
  []
  [mu]
  []
  [displacement]
  []
[]

[FluidProperties]
  [LEAD]
    type = LeadFluidProperties
  []
[]

[Problem]
  type = LiquidMetalSubChannel1PhaseProblem
  fp = LEAD
  n_blocks = 1
  P_out = 1.0e5
  CT = 1.0
  # enforce_uniform_pressure = false
  compute_density = true
  compute_viscosity = true
  compute_power = true
  P_tol = 1.0e-4
  T_tol = 1.0e-4
  implicit = true
  segregated = false
  staggered_pressure = false
  monolithic_thermal = false
  verbose_multiapps = true
  verbose_subchannel = false
[]

[ICs]
  [S_IC]
    type = TriFlowAreaIC
    variable = S
  []

  [w_perim_IC]
    type = TriWettedPerimIC
    variable = w_perim
  []

  [q_prime_IC]
    type = TriPowerIC
    variable = q_prime
    power = '${fparse 250000}'
    filename = "pin_power_profile19.txt"
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
    fp = LEAD
  []

  [rho_ic]
    type = RhoFromPressureTemperatureIC
    variable = rho
    p = ${P_out}
    T = T
    fp = LEAD
  []

  [h_ic]
    type = SpecificEnthalpyFromPressureTemperatureIC
    variable = h
    p = ${P_out}
    T = T
    fp = LEAD
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
[]

[Executioner]
  type = Steady
[]

# ################################################################################
# # A multiapp that projects data to a detailed mesh
# ################################################################################

# [MultiApps]
#   [viz]
#     type = FullSolveMultiApp
#     input_files = "3d_LBE_19.i"
#     execute_on = "timestep_end"
#   []
# []

# [Transfers]
#   [xfer]
#     type = MultiAppDetailedSolutionTransfer
#     to_multi_app = viz
#     variable = 'mdot SumWij P DP h T rho mu q_prime S'
#   []
# []
