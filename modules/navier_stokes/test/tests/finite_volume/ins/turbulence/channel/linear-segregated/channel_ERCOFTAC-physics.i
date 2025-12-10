
!include header_and_mesh.i

[Problem]
  linear_sys_names = 'u_system v_system pressure_system TKE_system TKED_system'
  previous_nl_solution_required = true
[]

[Physics]
  [NavierStokes]
    [FlowSegregated/flow]
      velocity_variable = 'vel_x vel_y'
      pressure_variable = 'pressure'

      # Initial conditions
      initial_velocity = '${bulk_u} 0 0'
      initial_pressure = '1e-8'

      # Material properties
      density = ${rho}
      dynamic_viscosity = ${mu}

      # Boundary conditions
      inlet_boundaries = 'left'
      momentum_inlet_types = 'fixed-velocity'
      momentum_inlet_functors = '${bulk_u} 0'

      wall_boundaries = 'top bottom'
      momentum_wall_types = 'noslip noslip'

      outlet_boundaries = 'right'
      momentum_outlet_types = 'fixed-pressure'
      pressure_functors = '0'

      # Numerical parameters
      include_deviatoric_stress = true
      orthogonality_correction = false
      pressure_two_term_bc_expansion = false
      momentum_two_term_bc_expansion = false
      momentum_advection_interpolation = ${advected_interp_method}
      system_names = 'u_system v_system pressure_system'
      # consistency with kernel syntax input, allows to use the same executioner block
      rhie_chow_uo_name = 'rc'
    []
    [TurbulenceSegregated/k-epsilon]
      # Model
      turbulence_handling = 'k-epsilon'
      tke_name = TKE
      tked_name = TKED
      system_names = 'TKE_system TKED_system'

      initial_tke = ${k_init}
      initial_tked = ${eps_init}

      # Model parameters
      mu_t_ratio_max = 1e20
      sigma_k = ${sigma_k}
      sigma_eps = ${sigma_eps}
      C_pl = 1e10
      C1_eps = ${C1_eps}
      C2_eps = ${C2_eps}

      turbulence_walls = ${walls}
      wall_treatment_eps = ${wall_treatment}
      bulk_wall_treatment = ${bulk_wall_treatment}
      use_nonorthogonal_correction = false
    []
  []
[]

# Boundary conditions are not implemented for turbulent quantities at this time
[LinearFVBCs]
  [inlet_TKE]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = TKE
    functor = '${k_init}'
  []
  [outlet_TKE]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right'
    variable = TKE
    use_two_term_expansion = false
  []
  [inlet_TKED]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = TKED
    functor = '${eps_init}'
  []
  [outlet_TKED]
    type = LinearFVAdvectionDiffusionOutflowBC
    boundary = 'right'
    variable = TKED
    use_two_term_expansion = false
  []
[]

!include executioner_regular_channel.i

variables_to_sample = 'vel_x vel_y pressure_over_density TKE TKED'
!include postprocessing.i
