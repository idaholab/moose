mu = 1.0
rho = 1.0e3
mu_d = 0.3
rho_d = 1.0
dp = 0.01
U_lid = 0.1
g = -9.81
velocity_interp_method = 'rc'
advected_interp_method = 'upwind'

k = 1
k_d = 1
cp = 1
cp_d = 1

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = .1
    ymin = 0
    ymax = .1
    nx = 10
    ny = 10
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [flow]
        compressibility = 'incompressible'

        density = 'rho_mixture'
        dynamic_viscosity = 'mu_mixture'

        # Initial conditions
        initial_velocity = '0 0 0'
        initial_pressure = 0

        # Pressure pin
        pin_pressure = true
        pinned_pressure_type = 'point-value-uo'
        pinned_pressure_point = '0 0 0'
        pinned_pressure_value = '0'

        # Gravity
        gravity = '0 ${g} 0'

        # Boundary conditions are defined outside of the Physics
        # Moving walls are not that common of a problem

        mass_advection_interpolation = '${advected_interp_method}'
        momentum_advection_interpolation = '${advected_interp_method}'
        velocity_interpolation = '${velocity_interp_method}'
      []
    []
    [TwoPhaseMixture]
      [mixture]
        phase_1_fraction_name = 'phase_1'
        phase_2_fraction_name = 'phase_2'

        add_phase_transport_equation = true
        phase_advection_interpolation = '${advected_interp_method}'
        phase_fraction_diffusivity = 1e-3

        # We could consider adding fixed-value-yet-not-an-inlet
        # boundary conditions to the TwoPhaseMixture physics

        # Base phase material properties
        phase_1_density_name = ${rho}
        phase_1_viscosity_name = ${mu}
        phase_1_specific_heat_name = ${cp}
        phase_1_thermal_conductivity_name = ${k}

        # Other phase material properties
        phase_2_density_name = ${rho_d}
        phase_2_viscosity_name = ${mu_d}
        phase_2_specific_heat_name = ${cp_d}
        phase_2_thermal_conductivity_name = ${k_d}
        output_all_properties = true

        # Friction model, not actually used!
        use_dispersed_phase_drag_model = true
        particle_diameter = ${dp}
      []
    []
  []
[]

[FVBCs]
  [top_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'top'
    function = ${U_lid}
  []
  [no_slip_x]
    type = INSFVNoSlipWallBC
    variable = vel_x
    boundary = 'left right bottom'
    function = 0
  []
  [no_slip_y]
    type = INSFVNoSlipWallBC
    variable = vel_y
    boundary = 'left right top bottom'
    function = 0
  []
  [bottom_phase_2]
    type = FVDirichletBC
    variable = phase_2
    boundary = 'bottom'
    value = 1.0
  []
  [top_phase_2]
    type = FVDirichletBC
    variable = phase_2
    boundary = 'top'
    value = 0.0
  []
[]

[AuxVariables]
  [drag_coefficient]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [populate_cd]
    type = FunctorAux
    variable = drag_coefficient
    functor = 'Darcy_coefficient'
    execute_on = 'TIMESTEP_END'
  []
[]

[Postprocessors]
  [average_void]
    type = ElementAverageValue
    variable = 'phase_2'
  []
  [max_y_velocity]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = max
  []
  [min_y_velocity]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = min
  []
  [max_x_velocity]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = max
  []
  [min_x_velocity]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = min
  []
  [max_x_slip_velocity]
    type = ElementExtremeFunctorValue
    functor = 'vel_slip_x'
    value_type = max
  []
  [max_y_slip_velocity]
    type = ElementExtremeFunctorValue
    functor = 'vel_slip_y'
    value_type = max
  []
  [max_drag_coefficient]
    type = ElementExtremeFunctorValue
    functor = 'drag_coefficient'
    value_type = max
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 7
    iteration_window = 2
    growth_factor = 2.0
    cutback_factor = 0.5
    dt = 1e-3
  []
  nl_max_its = 10
  nl_rel_tol = 1e-03
  nl_abs_tol = 1e-9
  l_max_its = 5
  end_time = 1e8
[]

[Outputs]
  exodus = false
  [CSV]
    type = CSV
    execute_on = 'FINAL'
  []
[]
