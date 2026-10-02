# Shared part of the friction factor sampling inputs. Each time step solves the assembly at a
# larger inlet mass flux, and the friction factor and Reynolds number of one interior, edge, and
# corner subchannel are reported. The including input defines the mesh, the problem type, the
# friction closure, the exponent G_exp0 of the initial inlet mass flux, and the center, edge, and
# corner subchannel indices.

T_in = 359.15 # K
P_out = 4.923e6 # Pa
# Inlet mass flux 10^(G_exp0 + 0.1 t) [kg/m^2-s]. The bulk Reynolds number is about 1 at t = 1 and
# grows by 0.1 decades per step, so end_time = 56 stops the sweep just below the upper limit
# Re = 3e5 of the Pacio-Chen-Todreas applicability range
G_exp_rate = 0.1

# The closure at the outlet node of a cell uses the Reynolds number at the inlet node, so the
# Reynolds number is sampled one cell (0.1 m) below the friction factor
z_Re = 0.4
z_ff = 0.5

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[SubChannel]
  fp = water
  n_blocks = 1
  P_out = ${P_out}
  compute_density = true
  compute_viscosity = true
  compute_power = false
  full_output = true
  mixing_closure = 'constant_beta'
  pin_HTC_closure = 'Dittus-Boelter'
[]

[SCMClosures]
  [Chen]
    type = SCMFrictionChenTodreas
  []
  [constant_beta]
    type = SCMMixingConstantBeta
    beta = 0.006
    CT = 1.8
  []
  [Dittus-Boelter]
    type = SCMHTCDittusBoelter
  []
[]

[ICs]
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
  [mu_ic]
    type = ViscosityIC
    variable = mu
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

[Functions]
  [mass_flux_fn]
    type = ParsedFunction
    expression = '10^(${G_exp0} + ${G_exp_rate} * t)'
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
    mass_flux = mass_flux
    execute_on = 'timestep_begin'
  []
[]

[Postprocessors]
  [mass_flux]
    type = FunctionValuePostprocessor
    function = mass_flux_fn
    execute_on = 'initial timestep_begin'
    outputs = none
  []
  [ff_center]
    type = SubChannelPointValue
    variable = ff
    index = ${center}
    height = ${z_ff}
  []
  [ff_edge]
    type = SubChannelPointValue
    variable = ff
    index = ${edge}
    height = ${z_ff}
  []
  [ff_corner]
    type = SubChannelPointValue
    variable = ff
    index = ${corner}
    height = ${z_ff}
  []
  [mdot_center]
    type = SubChannelPointValue
    variable = mdot
    index = ${center}
    height = ${z_Re}
    outputs = none
  []
  [mdot_edge]
    type = SubChannelPointValue
    variable = mdot
    index = ${edge}
    height = ${z_Re}
    outputs = none
  []
  [mdot_corner]
    type = SubChannelPointValue
    variable = mdot
    index = ${corner}
    height = ${z_Re}
    outputs = none
  []
  [w_perim_center]
    type = SubChannelPointValue
    variable = w_perim
    index = ${center}
    height = ${z_Re}
    outputs = none
  []
  [w_perim_edge]
    type = SubChannelPointValue
    variable = w_perim
    index = ${edge}
    height = ${z_Re}
    outputs = none
  []
  [w_perim_corner]
    type = SubChannelPointValue
    variable = w_perim
    index = ${corner}
    height = ${z_Re}
    outputs = none
  []
  [mu_center]
    type = SubChannelPointValue
    variable = mu
    index = ${center}
    height = ${z_Re}
    outputs = none
  []
  [mu_edge]
    type = SubChannelPointValue
    variable = mu
    index = ${edge}
    height = ${z_Re}
    outputs = none
  []
  [mu_corner]
    type = SubChannelPointValue
    variable = mu
    index = ${corner}
    height = ${z_Re}
    outputs = none
  []
  # Subchannel Reynolds number (mdot / S) Dh / mu with Dh = 4 S / w_perim, as in the solver
  [Re_center]
    type = ParsedPostprocessor
    expression = '4 * mdot_center / (w_perim_center * mu_center)'
    pp_names = 'mdot_center w_perim_center mu_center'
  []
  [Re_edge]
    type = ParsedPostprocessor
    expression = '4 * mdot_edge / (w_perim_edge * mu_edge)'
    pp_names = 'mdot_edge w_perim_edge mu_edge'
  []
  [Re_corner]
    type = ParsedPostprocessor
    expression = '4 * mdot_corner / (w_perim_corner * mu_corner)'
    pp_names = 'mdot_corner w_perim_corner mu_corner'
  []
[]

[Outputs]
  csv = true
[]

[Executioner]
  type = Transient
  start_time = 0.0
  end_time = 56
  dt = 1.0
[]
