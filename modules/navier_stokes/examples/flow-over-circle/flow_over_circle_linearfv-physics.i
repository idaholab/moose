!include header.i
!include mesh.i

[Physics]
  [NavierStokes]
    [FlowSegregated]
      [all]
        compressibility = incompressible

        dynamic_viscosity = ${mu}
        density = ${rho}

        initial_pressure = 0
        initial_velocity = '0 0 0'

        inlet_boundaries = 'left_boundary'
        momentum_inlet_types = 'fixed-velocity'
        momentum_inlet_functors = 'inlet_function 0.0'
        outlet_boundaries = 'right_boundary'
        momentum_outlet_types = 'fixed-pressure'
        pressure_functors = 0.0
        wall_boundaries = 'top_boundary bottom_boundary circle'
        momentum_wall_types = 'noslip noslip noslip'

        momentum_advection_interpolation = ${advected_interp_method}
      []
    []
  []
[]

[Functions]
  [inlet_function]
    type = ParsedFunction
    expression = '4*U*(y-ymin)*(ymax-y)/(ymax-ymin)/(ymax-ymin)'
    symbol_names = 'U ymax ymin'
    symbol_values = '${inlet_velocity} ${y_max} ${y_min}'
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
[]

[Executioner]
  type = PIMPLE
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  momentum_l_tol = 1e-12
  pressure_l_tol = 1e-12
  rhie_chow_user_object = 'ins_rhie_chow_interpolator'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.90
  pressure_variable_relaxation = 0.4
  num_iterations = 100
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
  dt = 0.01
  num_steps = 500
[]

[Outputs]
  exodus = true
  csv = true
[]

!include postprocessors.i
