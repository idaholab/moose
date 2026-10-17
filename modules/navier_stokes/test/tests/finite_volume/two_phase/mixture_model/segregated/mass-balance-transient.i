##########################################################
# Mass balance of the mixture under a falling outlet pressure.
#
# Mixture continuity is d(rho_m)/dt + div(rho_m u_m) = 0. The pressure equation of the segregated
# algorithm realises the divergence alone, so without the storage term the solver forces the mass
# flux in to equal the mass flux out at every instant and the domain cannot accumulate or release
# mass. Here the outlet pressure is ramped down, the gas expands, and the mixture mass held in the
# pipe genuinely falls: the imbalance is then the whole of the missing term rather than a
# correction to it.
#
# The assertion is
#
#   d/dt Int(rho_m dV)  =  mdot_in - mdot_out
#
# which 'mass_imbalance' below forms directly, normalised by the inlet mass flow. With the storage
# term off it is order one. With it on and the sign right it closes to the solver tolerance; with
# the sign wrong it doubles rather than closing, which is what makes this a test of the sign and
# not only of the magnitude.
##########################################################

# Air and water at 0.1 MPa
rho_f = 998.0
mu_f = 1.0e-3
# rho_g is the gas density at the reference pressure p_ref, which is the pressure the outlet
# boundary condition holds. The density at any other point follows from the solved pressure.
rho_g = 1.2
p_outlet_gauge = 5950   # 59.5 mBar, taken as gauge
p_ref = '${fparse 101325 + p_outlet_gauge}'
mu_g = 1.8e-5
# Weight of the mixture column assumed by the 'imposed' density model below. Unused by the others.
rho_m_col = 948

# Geometry
D = 0.052
R = '${fparse D / 2}'
L = 3.9676     # z/D = 22.4 to 98.7

# Flow condition, the first of the low void fraction cases of the reference report
# Conditions from Pena-Monferrer et al., Sci. Technol. Nucl. Install. 2018, 2153019, Table 1,
# which is the source of the data Wu et al. compare against. Liquid velocity 0.5 m/s throughout.
#
#   case      j_g (m/s)   alpha    bubble mean d (mm)   sigma (mm)   Sauter d32 (mm)
#   PW05002    0.01962    0.0215        2.777             0.602          3.026
#   PW05003    0.03001    0.0338        2.760             0.643          3.044
#   PW05004    0.04012    0.0450        2.976             0.577          3.192
#
# The bubble size distributions are normal fits over about 500 bubbles per port. The drag law wants
# a Sauter mean rather than an arithmetic one, so d32 = (mu^3 + 3 mu sigma^2)/(mu^2 + sigma^2) is
# formed from the reported statistics and used as 'dp'.
#
# The reference measures bubbles growing by roughly 30% in volume between the bottom and top ports,
# by decompression. 'dp' here is held constant at its inlet value, so that growth is not carried.
# Whether it matters is checked by running the case at the inlet and outlet diameters; the drag law
# in the distorted particle regime is close to independent of diameter, so it is expected not to.
#
# Run the other two cases by overriding alpha_in and dp together.
alpha_in = 0.0215
# The reference sets the inlet mixture velocity equal to the inlet liquid velocity of 0.5 m/s,
# because a drift-flux model solves for the mixture velocity rather than the phase velocities.
u_in = 0.5
# Bubble diameter. Not reported by the reference; a parameter of this study, see the header.
dp = 0.003026
sigma = 0.0728
drag_model = 'schiller-naumann'
swarm_exponent = 0.0


# Required by the mixture Physics but not used: there is no energy equation here
cp_f = 1
k_f = 1
cp_g = 1
k_g = 1

[Mesh]
  coord_type = RZ
  rz_coord_axis = X
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = ${L}
    ymin = 0
    ymax = ${R}
    nx = 40
    ny = 4
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system phi_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [FlowSegregated/flow]
      compressibility = 'incompressible'
      density = 'rho_mixture'
      dynamic_viscosity = 'mu_mixture'
      gravity = '-9.81 0 0'

      initial_velocity = '${u_in} 0 0'
      initial_pressure = 0

      inlet_boundaries = 'left'
      momentum_inlet_types = 'fixed-velocity'
      momentum_inlet_functors = '${u_in} 0'

      # 'top' is the pipe wall, 'bottom' is the axis of symmetry
      wall_boundaries = 'top bottom'
      momentum_wall_types = 'noslip symmetry'

      outlet_boundaries = 'right'
      momentum_outlet_types = 'fixed-pressure'
      pressure_functors = 'p_out_ramp'

      momentum_advection_interpolation = 'upwind'
      orthogonality_correction = false
      pressure_two_term_bc_expansion = true
      momentum_two_term_bc_expansion = true
    []
    [TwoPhaseMixtureSegregated/mixture]
      system_names = 'phi_system'
      phase_1_fraction_name = 'phase_1'
      phase_2_fraction_name = 'phase_2'

      add_phase_transport_equation = true
      add_mass_density_transient = true
      phase_advection_interpolation = 'upwind'
      phase_fraction_inlet_type = 'fixed-value'
      phase_fraction_inlet_functors = '${alpha_in}'

      # Drift flux
      add_drift_flux_momentum_terms = true
      density_interp_method = 'average'

      # Continuous phase is water, dispersed phase is air
      phase_1_density_name = ${rho_f}
      phase_1_viscosity_name = ${mu_f}
      phase_1_specific_heat_name = ${cp_f}
      phase_1_thermal_conductivity_name = ${k_f}

      phase_2_density_name = 'rho_g_var'
      phase_2_density_time_derivative = 'drho_g_dt'
      phase_2_viscosity_name = ${mu_g}
      phase_2_specific_heat_name = ${cp_g}
      phase_2_thermal_conductivity_name = ${k_g}

      use_dispersed_phase_drag_model = true
      slip_drag_model = ${drag_model}
      surface_tension = ${sigma}
      slip_swarm_exponent = ${swarm_exponent}
      particle_diameter = ${dp}
      add_advection_slip_term = false
    []
  []
[]

# The radial momentum equation needs the axisymmetric hoop term, which the Physics does not add


[LinearFVKernels]
  # Interfacial area concentration, Fluent 16.4-27 in conservative form

  [radial_rz_viscous]
    type = LinearFVRZViscousSource
    variable = vel_y
    mu = 'mu_mixture'
    momentum_component = 'y'
  []
[]

[Functions]
  # 2 kPa/s fall, so 20 kPa over the run: about a fifth of the absolute pressure, enough that the
  # expansion is unmistakable without approaching the floor in the gas density.
  [p_out_ramp]
    type = ParsedFunction
    expression = '-2000 * t'
  []
[]

[FunctorMaterials]
  # Only one of the three density models below is active. They differ solely in where the pressure
  # that sets the gas density comes from, which is the comparison section 8 of the report makes.
  # Select one from the command line, e.g.
  #   FunctorMaterials/active='rho_g_imposed c_d j_axial alpha_j alpha_v_gj'
  active = 'rho_g_solved p_dot drho_g_dt c_d j_axial alpha_j alpha_v_gj'

  # Gas density from the solved pressure, isothermal ideal gas: rho_g = rho_g(p_ref) p_abs / p_ref.
  #
  # 'pressure' is the total pressure the flow Physics solves, not a dynamic pressure: gravity enters
  # the momentum equation as a full rho_m g body force (solve_for_dynamic_pressure defaults to
  # false), so the solved field already carries the hydrostatic head, gauged to zero at the outlet.
  # The absolute pressure is therefore p_ref + pressure.
  #
  # This closes the loop that an imposed rho_g(x) leaves open. The column is weighed by whatever
  # rho_mixture the solution produces rather than by an assumed constant, so the expansion is
  # computed rather than prescribed. The flow solver is still incompressible -- the volume created
  # by the expansion does not accelerate u_m -- but the conservative phase equation carries rho_d,
  # so dispersed phase mass is conserved and alpha grows as rho_g falls.
  #
  # The floor guards the first outer iterations, where the pressure field is still developing from
  # its zero initial condition and can undershoot; it is inactive in the converged solution.
  [rho_g_solved]
    type = ParsedFunctorMaterial
    property_name = 'rho_g_var'
    expression = '${rho_g} * max(${p_ref} + pressure, 0.1 * ${p_ref}) / ${p_ref}'
    functor_names = 'pressure'
  []

  # d(rho_d)/dt for the phase equation. rho_d depends on time only through the pressure, so the
  # chain rule gives d(rho_d)/dt = (d rho_d/dp) dp/dt, and dp/dt comes from the pressure variable's
  # own time derivative, which is built from the time integrator rather than differenced by hand.
  [p_dot]
    type = GenericFunctorTimeDerivativeMaterial
    prop_names = 'p_dot'
    prop_values = 'pressure'
  []
  [drho_g_dt]
    type = ParsedFunctorMaterial
    property_name = 'drho_g_dt'
    expression = '${rho_g} / ${p_ref} * p_dot'
    functor_names = 'p_dot'
  []

  # Gas density from a prescribed hydrostatic column of fixed weight. This presupposes the void
  # fraction, since rho_m_col is the mixture density the answer is supposed to produce, and it gives
  # every void fraction the same expansion ratio. Retained for comparison only.
  [rho_g_imposed]
    type = ParsedFunctorMaterial
    property_name = 'rho_g_var'
    expression = '${rho_g} * (${p_ref} + ${rho_m_col} * 9.81 * (${L} - x)) / ${p_ref}'
  []

  # Gas density held at its reference value. The bubbles do not expand, which isolates how much of
  # the measured interfacial area development the coalescence and breakage terms can supply alone.
  [rho_g_constant]
    type = ParsedFunctorMaterial
    property_name = 'rho_g_var'
    expression = '${rho_g}'
  []

  # The drift-flux quantities of interest, built up in order. Each is a plain functor of the
  # solved fields and of the slip velocity the closure produces.
  [c_d]
    type = ParsedFunctorMaterial
    property_name = 'c_d'
    expression = 'phase_2 * rho_g_var / rho_mixture'
    functor_names = 'phase_2 rho_g_var rho_mixture'
  []
  [j_axial]
    type = ParsedFunctorMaterial
    property_name = 'j_axial'
    expression = 'vel_x + (phase_2 - c_d) * vel_slip_x'
    functor_names = 'vel_x phase_2 c_d vel_slip_x'
  []
  [j_gas]
    type = ParsedFunctorMaterial
    property_name = 'j_gas'
    expression = 'phase_2 * (vel_x + (1 - c_d) * vel_slip_x)'
    functor_names = 'phase_2 vel_x c_d vel_slip_x'
  []
  [alpha_j]
    type = ParsedFunctorMaterial
    property_name = 'alpha_j'
    expression = 'phase_2 * j_axial'
    functor_names = 'phase_2 j_axial'
  []
  [alpha_v_gj]
    type = ParsedFunctorMaterial
    property_name = 'alpha_v_gj'
    expression = 'phase_2 * (1 - phase_2) * vel_slip_x'
    functor_names = 'phase_2 vel_slip_x'
  []
[]

[Executioner]
  # short: the balance closes from the first step, so the test does not need a developed flow
  type = PIMPLE
  num_steps = 10
  dt = 0.02
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  active_scalar_systems = 'phi_system'

  momentum_equation_relaxation = 0.7
  active_scalar_equation_relaxation = '0.7'
  pressure_variable_relaxation = 0.3

  num_iterations = 30
  pressure_absolute_tolerance = 1e-9
  momentum_absolute_tolerance = 1e-9
  active_scalar_absolute_tolerance = '1e-9'
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  active_scalar_petsc_options_iname = '-pc_type -pc_hypre_type'
  active_scalar_petsc_options_value = 'hypre boomeramg'
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  active_scalar_l_abs_tol = 1e-12
  continue_on_max_its = true
[]

[Postprocessors]
  [alpha_in_pp]
    type = SideAverageFunctorPostprocessor
    functor = phase_2
    boundary = 'left'
  []
  [alpha_out]
    type = SideAverageFunctorPostprocessor
    functor = phase_2
    boundary = 'right'
  []
  # The mixture mass held in the pipe, and the mass flows across its two ends. VolumetricFlowRate
  # with an advected quantity of rho_mixture uses the Rhie-Chow face flux, so these are the same
  # fluxes the pressure equation is built from rather than a reconstruction of them.
  [mixture_mass]
    type = ElementIntegralFunctorPostprocessor
    functor = rho_mixture
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [d_mass]
    type = ChangeOverTimePostprocessor
    postprocessor = mixture_mass
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [mdot_in]
    type = VolumetricFlowRate
    boundary = 'left'
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = 'rho_mixture'
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    # the Rhie-Chow face flux map is not populated until the first solve
    execute_on = 'TIMESTEP_END'
  []
  [mdot_out]
    type = VolumetricFlowRate
    boundary = 'right'
    vel_x = vel_x
    vel_y = vel_y
    advected_quantity = 'rho_mixture'
    rhie_chow_user_object = 'ins_rhie_chow_interpolator'
    # the Rhie-Chow face flux map is not populated until the first solve
    execute_on = 'TIMESTEP_END'
  []
  # d/dt Int(rho_m dV) - (mdot_in - mdot_out), normalised by the inlet mass flow
  [mass_imbalance]
    type = ParsedPostprocessor
    expression = '(d_mass / 0.02 + mdot_in + mdot_out) / abs(mdot_in)'
    pp_names = 'd_mass mdot_in mdot_out'
  []
  [min_alpha]
    type = ElementExtremeValue
    variable = phase_2
    value_type = min
  []
  [max_alpha]
    type = ElementExtremeValue
    variable = phase_2
  []

  # Evidence that the expansion is computed rather than imposed: the inlet gauge pressure is the
  # weight of the mixture above it, and the ratio is the expansion the gas undergoes along the pipe.
  [p_inlet]
    type = SideAverageValue
    variable = pressure
    boundary = 'left'
  []
  [rho_g_inlet]
    type = SideAverageFunctorPostprocessor
    functor = rho_g_var
    boundary = 'left'
  []
  [rho_g_outlet]
    type = SideAverageFunctorPostprocessor
    functor = rho_g_var
    boundary = 'right'
  []
  [rho_g_ratio]
    type = ParsedPostprocessor
    expression = 'rho_g_inlet / rho_g_outlet'
    pp_names = 'rho_g_inlet rho_g_outlet'
  []
[]

[Outputs]
  csv = true
[]
