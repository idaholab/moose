### Thermophysical Properties ###
rho = 1
mu = 100

### Simulation parameters
inlet_velocity = 1
side_length = 1
restart_initial_dt = 1
[Mesh]
  active = 'gen'
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = 0
    ymax = ${side_length}
    nx = 100
    ny = 20
  []
  [fmg_restart]
    type = FileMeshGenerator
    file = 2d_channel_init_exodus.e
    use_for_exodus_restart = true
  []
[]

[Physics]
  [NavierStokes]
    [Flow]
      [all_flow]
        compressibility = 'incompressible'

        density = ${rho}
        dynamic_viscosity = ${mu}

        inlet_boundaries = 'left'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = '${inlet_velocity} 0'

        wall_boundaries = 'top bottom'
        momentum_wall_types = 'noslip noslip'

        outlet_boundaries = 'right'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = '0'

        # Make sure restart is perfect by freezing the time step dependence of
        # the contribution to the RC coefficient by the time derivative kernel
        time_derivative_rc_coef_fixed_dt = ${restart_initial_dt} # (s)
        time_derivative_rc_coef_fixed_dt_start = 10

        mass_advection_interpolation = 'upwind'
        momentum_advection_interpolation = 'upwind'
      []
    []
  []
[]

[Functions]
  [grow_dt]
    type = PiecewiseLinear
    x = '0 10'
    y = '0.1 10'
  []
  [constant]
    type = PiecewiseConstant
    x = '0 10'
    y = '${restart_initial_dt} ${restart_initial_dt}'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  nl_abs_tol = 1e-10

  line_search = 'none'
  [TimeStepper]
    type = FunctionDT
    function = 'grow_dt'
  []
  # Enough to obtain steady state (no nonlinear iteration needed)
  end_time = 40
[]

[Outputs]
  # Used to set up a restart from exodus file
  [exodus]
    type = Exodus
    execute_on = TIMESTEP_END
  []
  # Used to check results
  csv = true
[]

[Postprocessors]
  [min_vel_x]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = 'min'
  []
  [max_vel_x]
    type = ElementExtremeValue
    variable = 'vel_x'
    value_type = 'max'
  []
  [min_vel_y]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = 'min'
  []
  [max_vel_y]
    type = ElementExtremeValue
    variable = 'vel_y'
    value_type = 'max'
  []
  [min_pressure]
    type = ElementExtremeValue
    variable = 'pressure'
    value_type = 'min'
  []
  [max_pressure]
    type = ElementExtremeValue
    variable = 'pressure'
    value_type = 'max'
  []
[]
