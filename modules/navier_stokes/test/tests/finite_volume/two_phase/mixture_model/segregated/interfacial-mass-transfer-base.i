# Common part of the interfacial mass transfer verifications. Not run on its own.
#
# A closed, uniform, quiescent box of mixture with equal phase densities and specific heats, in
# which the interfacial mass transfer generates the dispersed phase and absorbs its latent heat
# from the energy equation. Every field stays spatially uniform, so each including input reduces to
# ordinary differential equations with an exact answer. What the including input supplies is the
# transfer rate: closed on the transported interfacial area, or prescribed as a constant. It sets
# 'gamma_name' to the functor carrying that rate before including this file.
#
# Continuous phase, liquid
rho = 1000.0
mu = 1e-3
cp = 4000.0
k = 0.6

# Dispersed phase. Its density and specific heat are deliberately those of the continuous phase,
# which is what makes this case well posed and what makes it measure one thing.
#
# Equal densities. A closed rigid box cannot accommodate a transfer between phases of different
# density: the mass it holds is fixed, so converting one phase into the other at fixed phase
# densities would demand a change of volume there is nowhere to take. With the two densities equal
# the mixture density is constant, mass and volume are both conserved, and the dilatation the
# relative motion would produce vanishes identically because c_d = alpha.
#
# Equal specific heats. The mixture specific heat is then constant in time, so the energy equation
# needs no time derivative of rho_m cp_m, and the enthalpy the relative motion carries vanishes
# because it is proportional to the difference of the two. What is left driving the energy equation
# is the latent heat of the transfer alone.
#
# The density is constant in time either way, which is what makes the invariant exact rather than
# approximate.
rho_d = 1000.0
mu_d = 1e-5
cp_d = 4000.0
k_d = 0.03

# Latent heat of the transition. Far smaller than any real one, chosen so that the phase fraction
# changes by a large factor over a handful of steps: the assertions below are checked against a
# change large enough to distinguish the right exponent or slope from a neighbouring one, which a
# physical latent heat would leave at the level of the solver tolerance.
h_fg = 1e4

# Initial state. The superheat drives the transfer.
T_initial = 375.0
alpha_initial = 0.05
area_initial = 300.0

t_end = 2.0

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 0.1
    ymin = 0
    ymax = 0.1
    nx = 4
    ny = 4
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system phase_system energy_system xi_system'
[]

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [flow]
        compressibility = 'weakly-compressible'
        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'

        initial_velocity = '1e-12 1e-12 0'
        initial_pressure = 0

        wall_boundaries = 'top left right bottom'
        momentum_wall_types = 'noslip noslip noslip noslip'
        momentum_wall_functors = '0 0; 0 0; 0 0; 0 0'

        orthogonality_correction = false
        pressure_two_term_bc_expansion = true
        momentum_advection_interpolation = 'upwind'
      []
    []
    [FluidHeatTransferSegregated]
      [energy]
        system_names = 'energy_system'
        thermal_conductivity = 'k_mixture'
        specific_heat = 'cp_mixture'
        initial_temperature = ${T_initial}

        energy_wall_boundaries = 'top left right bottom'
        energy_wall_types = 'heatflux heatflux heatflux heatflux'
        energy_wall_functors = '0 0 0 0'
      []
    []
    [TwoPhaseMixtureSegregated]
      [mixture]
        system_names = 'phase_system'
        fluid_heat_transfer_physics = 'energy'

        phase_1_fraction_name = 'phase_1'
        phase_2_fraction_name = 'phase_2'
        initial_phase_fraction = ${alpha_initial}

        phase_1_density_name = ${rho}
        phase_1_viscosity_name = ${mu}
        phase_1_specific_heat_name = ${cp}
        phase_1_thermal_conductivity_name = ${k}

        phase_2_density_name = ${rho_d}
        phase_2_viscosity_name = ${mu_d}
        phase_2_specific_heat_name = ${cp_d}
        phase_2_thermal_conductivity_name = ${k_d}

        use_dispersed_phase_drag_model = true
        particle_diameter = 1e-3
      []
    []
  []
[]

[FunctorMaterials]
  # The factor the energy time derivative is built on. The mixture specific heat the Physics
  # creates is the mass-weighted average, so this product is the mixture enthalpy density per unit
  # temperature.
  [rho_cp]
    type = ParsedFunctorMaterial
    property_name = 'rho_cp'
    expression = 'rho_mixture * cp_mixture'
    functor_names = 'rho_mixture cp_mixture'
  []
[]

[Variables]
  [interface_area]
    type = MooseLinearVariableFVReal
    solver_sys = xi_system
    initial_condition = ${area_initial}
  []
[]

# The interfacial area transport equation, written out here because no Physics assembles it. With
# no flow the advection contributes nothing, so the transient balances the sources.
[LinearFVKernels]
  [area_time]
    type = LinearFVTimeDerivative
    variable = interface_area
    factor = ${rho_d}
    conservative_form = true
  []
  [area_sources]
    type = LinearWCNSFV2PInterfaceAreaSourceSink
    variable = interface_area
    model = 'hibiki-ishii'

    u = vel_x
    v = vel_y

    rho_d = ${rho_d}
    rho_f = ${rho}
    fraction_dispersed = 'phase_2'
    sigma = 0.059
    epsilon = 1e-6

    # The one rate the phase equation is driven by, named by the input that includes this file
    mass_transfer_rate = ${gamma_name}

    # Zeroing both interaction coefficients leaves the phase change term as the only source, which
    # is what the invariant in the header was derived for. Hibiki and Ishii has no wake
    # entrainment, so this switches off every bubble interaction.
    gamma_c = 0
    gamma_b = 0
  []
[]

[LinearFVBCs]
[]

[Executioner]
  type = PIMPLE
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'

  dt = 0.2
  end_time = ${t_end}

  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  active_scalar_systems = 'phase_system xi_system'

  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.9
  active_scalar_equation_relaxation = '0.9 0.9'

  num_iterations = 200
  pressure_absolute_tolerance = 1e-11
  momentum_absolute_tolerance = 1e-11
  energy_absolute_tolerance = 1e-11
  active_scalar_absolute_tolerance = 1e-11

  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  energy_l_abs_tol = 1e-13
  active_scalar_l_abs_tol = 1e-13
  momentum_l_tol = 1e-12
  pressure_l_tol = 1e-12
  energy_l_tol = 1e-12
  active_scalar_l_tol = 1e-12
  continue_on_max_its = true

  pin_pressure = true
  pressure_pin_value = 0.0
  pressure_pin_point = '0.0 0.0 0.0'

  print_fields = false
[]

[Postprocessors]
  [alpha]
    type = ElementAverageValue
    variable = phase_2
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [area]
    type = ElementAverageValue
    variable = interface_area
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [temperature]
    type = ElementAverageValue
    variable = T_fluid
    execute_on = 'INITIAL TIMESTEP_END'
  []
  # chi / alpha^(2/3), constant under the phase change term alone. See the header.
  [number_invariant]
    type = ParsedPostprocessor
    expression = 'area / alpha^(2.0/3.0)'
    pp_names = 'area alpha'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
[]
