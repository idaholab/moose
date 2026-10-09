# Shared part of the inputs that reproduce the calculations of the PCTD paper, Pacio et al. (2022),
# for the wire-wrapped triangular lattices of Kennedy et al. (2015) and Liang et al. (2020). Each
# time step solves the assembly at a larger inlet mass flux, which sets the bulk Reynolds number.
# The including input defines the mesh, the exponent G_exp0 of the initial inlet mass flux, the
# number of steps, the center, edge, and corner subchannel indices and counts, and includes the file
# with the mass flow rate and flow area of every subchannel at the outlet. The PCTD model lumps the
# subchannels of each type, so the flow split of a type is calculated from those in the plotting
# script. The bulk friction factor is calculated from the pressure gradient in the developed flow.
# The fluid is water, since the flow split and friction factors depend on the bulk Reynolds number,
# not on the fluid.

T_in = 359.15 # K
P_out = 4.923e6 # Pa
# The inlet mass flux is uniform, and the flow split develops over about 1.5 m, so it is sampled at
# the outlet of the 3 m assembly, where the flow is developed
z_out = 3.0
# The pressure gradient is calculated between z_dp_in and z_dp_out, in the developed flow and away
# from the outlet boundary
z_dp_in = 2.0
z_dp_out = 2.9
# Inlet mass flux 10^(G_exp0 + G_exp_rate n) [kg/m^2-s] at step n
G_exp_rate = 0.1
# Each sweep step is a time step of the transient solve; the time derivative of the axial momentum
# equation is negligible for a time step this long, as in friction_factor_sampling.i
dt_step = 1e4 # s

[FluidProperties]
  [water]
    type = Water97FluidProperties
  []
[]

[SubChannel]
  type = TriSubChannel1PhaseProblem
  fp = water
  n_blocks = 1
  P_out = ${P_out}
  compute_density = true
  compute_viscosity = true
  compute_power = false
  full_output = true
  # Monolithic implicit solver. The explicit solver converges the developed turbulent flow split
  # only to about 1% with the default tolerances.
  implicit = true
  segregated = false
  P_tol = 1e-8
  T_tol = 1e-8
  rtol = 1e-10
  atol = 1e-10
  # No gravity, so that the pressure gradient is the friction pressure gradient
  gravity = none
  friction_closure = 'Chen'
  mixing_closure = 'Chen_Todreas'
  pin_HTC_closure = 'Dittus-Boelter'
[]

[SCMClosures]
  [Chen]
    type = SCMFrictionChenTodreas
    friction_model = Pacio
  []
  [Chen_Todreas]
    type = SCMMixingChenTodreas
    mixing_model = Pacio
    # C_T multiplies the mixing parameter in the axial momentum equation, whose mixing term is the
    # exchange of axial momentum of the PCTD equations, Eq. (2) of Pacio et al. (2022); the enthalpy
    # mixing uses the mixing parameter without C_T. C_T = 1 applies it unchanged, as in PCTD.
    CT = 1.0
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
    expression = '10^(${G_exp0} + ${G_exp_rate} * t / ${dt_step})'
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
  []
  [w_perim_center]
    type = SubChannelPointValue
    variable = w_perim
    index = ${center}
    height = ${z_out}
    outputs = none
  []
  [w_perim_edge]
    type = SubChannelPointValue
    variable = w_perim
    index = ${edge}
    height = ${z_out}
    outputs = none
  []
  [w_perim_corner]
    type = SubChannelPointValue
    variable = w_perim
    index = ${corner}
    height = ${z_out}
    outputs = none
  []
  # The solver evaluates the bulk Reynolds number with the inlet viscosity
  [mu_in]
    type = SubChannelPointValue
    variable = mu
    index = ${center}
    height = 0.0
    outputs = none
  []
  # Subchannel hydraulic diameters Dh = 4 S / w_perim, as in the solver
  [Dh_center]
    type = ParsedPostprocessor
    expression = '4 * S_${center} / w_perim_center'
    pp_names = 'S_${center} w_perim_center'
  []
  [Dh_edge]
    type = ParsedPostprocessor
    expression = '4 * S_${edge} / w_perim_edge'
    pp_names = 'S_${edge} w_perim_edge'
  []
  [Dh_corner]
    type = ParsedPostprocessor
    expression = '4 * S_${corner} / w_perim_corner'
    pp_names = 'S_${corner} w_perim_corner'
  []
  # Bulk hydraulic diameter, 4 times the total flow area over the total wetted perimeter
  [Dh_bulk]
    type = ParsedPostprocessor
    expression = '4 * (${n_center} * S_${center} + ${n_edge} * S_${edge} + ${n_corner} * S_${corner})
                  / (${n_center} * w_perim_center + ${n_edge} * w_perim_edge
                     + ${n_corner} * w_perim_corner)'
    pp_names = 'S_${center} S_${edge} S_${corner} w_perim_center w_perim_edge w_perim_corner'
  []
  [Re_bulk]
    type = ParsedPostprocessor
    expression = 'mass_flux * Dh_bulk / mu_in'
    pp_names = 'mass_flux Dh_bulk mu_in'
  []
  # Bulk friction factor f_b = (dP/L) 2 Dh_bulk rho / mass_flux^2, from the pressure gradient of the
  # developed flow
  [P_dp_in]
    type = SubChannelPointValue
    variable = P
    index = ${center}
    height = ${z_dp_in}
    outputs = none
  []
  [P_dp_out]
    type = SubChannelPointValue
    variable = P
    index = ${center}
    height = ${z_dp_out}
    outputs = none
  []
  [rho_dp]
    type = SubChannelPointValue
    variable = rho
    index = ${center}
    height = ${z_dp_in}
    outputs = none
  []
  [f_bulk]
    type = ParsedPostprocessor
    expression = '(P_dp_in - P_dp_out) / (z_dp_out - z_dp_in) * 2 * Dh_bulk * rho_dp / mass_flux^2'
    constant_names = 'z_dp_in z_dp_out'
    constant_expressions = '${z_dp_in} ${z_dp_out}'
    pp_names = 'P_dp_in P_dp_out Dh_bulk rho_dp mass_flux'
  []
[]

[Outputs]
  csv = true
[]

[Executioner]
  type = Transient
  start_time = 0.0
  end_time = '${fparse n_steps * dt_step}'
  dt = ${dt_step}
[]
