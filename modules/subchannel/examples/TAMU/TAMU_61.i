# Vaghetto, R., Jones, P., Goth, N., Childs, M., Lee, S., Thien Nguyen, D. and Hassan, Y.A., 2018.
# Pressure measurements in a wire-wrapped 61-pin hexagonal fuel bundle.
# Journal of Fluids Engineering, 140(3), p.031104.
T_in = 300
flow_area = 0.00799219 #m2
Dh = 0.00768504 #m
mass_flux_in = '${fparse 23.86/flow_area}' # [kg/sm2]
P_out = 2.0e5 # Pa
[TriSubChannelMesh]
  [subchannel]
    type = TriSubChannelMeshGenerator
    nrings = 5
    n_cells = 40
    flat_to_flat = 0.154
    heated_length = 2
    rod_diameter = 1.59e-2
    pitch = 1.89e-2
    dwire = 3.0e-3
    hwire = 0.476
    spacer_z = '0'
    spacer_k = '0'
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
  [water]
    type = PBSodiumFluidProperties
  []
[]

[Problem]
  type = TriSubChannel1PhaseProblem
  fp = water
  n_blocks = 1
  P_out = ${P_out}
  CT = 2.6
  compute_density = true
  compute_viscosity = true
  compute_power = false
  P_tol = 1.0e-4
  T_tol = 1.0e-4
  implicit = true
  segregated = false
  staggered_pressure = false
  monolithic_thermal = false
  verbose_multiapps = true
  verbose_subchannel = true
  interpolation_scheme = 'upwind'
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
    power = 10000 #W
    filename = "pin_power_profile61.txt"
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
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [T]
    type = SubChannelPointValue
    variable = T
    index = 36
    execute_on = "timestep_end"
    height = 0.7
  []

  [Pin_Planar_Mean]
    type = PlanarMean
    variable = P
    execute_on = 'TIMESTEP_END'
    height = 0.0
  []

  [Pout_Planar_Mean]
    type = PlanarMean
    variable = P
    execute_on = 'TIMESTEP_END'
    height = 2.0
  []

  [Pout_user_provided]
    type = Receiver
    default = ${P_out}
    execute_on = 'TIMESTEP_END'
  []

  ### Assembly inlet density
  [rho_in]
    type = PlanarMean
    variable = rho
    execute_on = 'TIMESTEP_END'
    height = 0.0
  []

  ####### Assembly pressure drop
  [DP_Planar_mean]
    type = ParsedPostprocessor
    pp_names = 'Pin_Planar_Mean Pout_Planar_Mean'
    function = 'Pin_Planar_Mean - Pout_Planar_Mean'
  []
  [DP_SubchannelDelta]
    type = SubChannelDelta
    variable = P
    execute_on = 'TIMESTEP_END'
  []
  ######
  [Kloss]
    type = ParsedPostprocessor
    pp_names = 'DP_SubchannelDelta rho_in'
    function = '2.0 * DP_SubchannelDelta * rho_in  / (${mass_flux_in} * ${mass_flux_in})'
  []

  [core_f]
    type = ParsedPostprocessor
    pp_names = 'Kloss'
    function = 'Kloss * ${Dh} / 2.0'
  []
[]
