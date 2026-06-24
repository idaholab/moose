rho = 1
u_inlet = 1
height = 1
mu = 0.02
nx = 12
ny = 3
advected_interp_method = 'average'
momentum_relaxation = 0.5
pressure_relaxation = 1.0

# Reference pressure drop computed from this input with advected_interp_method = average,
# nx = 192, ny = 48, use_two_term_expansion = true on the velocity outflow and
# pressure extrapolation BCs, and pressure/momentum absolute tolerances of 1e-10. The
# convergence test compares the absolute pressure-drop deviation from this stored
# refined value.
reference_pressure_drop = 0.8386252

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 5
    ymin = 0
    ymax = ${height}
    nx = ${nx}
    ny = ${ny}
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
    pressure_projection_method = CONSISTENT
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = ${u_inlet}
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    initial_condition = 0
    solver_sys = v_system
  []
  [pressure]
    type = MooseLinearVariableFVReal
    initial_condition = 0
    solver_sys = pressure_system
  []
[]

[Functions]
  [inlet_u]
    type = ParsedFunction
    expression = '30 * ${u_inlet} * y^2 * (${height} - y)^2 / ${height}^4'
  []
[]

[FVInterpolationMethods]
  [average]
    type = FVGeometricAverage
  []
  [upwind]
    type = FVAdvectedUpwind
  []
  [vanLeer]
    type = FVAdvectedVanLeerWeightBased
  []
  [min_mod]
    type = FVAdvectedMinmodWeightBased
  []
  [venkatakrishnan]
    type = FVAdvectedVenkatakrishnanDeferredCorrection
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method_name = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method_name = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = 'y'
  []
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
[]

[LinearFVBCs]
  [inlet-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_x
    functor = inlet_u
  []
  [inlet-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_y
    functor = 0
  []
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top bottom'
    variable = vel_x
    functor = 0
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top bottom'
    variable = vel_y
    functor = 0
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right'
    variable = pressure
    functor = 0
  []
  [inlet_p]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'left'
    variable = pressure
    use_two_term_expansion = true
  []
  [walls_p]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'top bottom'
    variable = pressure
    use_two_term_expansion = true
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = true
    boundary = right
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_y
    use_two_term_expansion = true
    boundary = right
  []
[]

[Executioner]
  type = SIMPLE
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'

  momentum_equation_relaxation = ${momentum_relaxation}
  pressure_variable_relaxation = ${pressure_relaxation}
  num_iterations = 2000
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  momentum_l_abs_tol = 1e-10
  pressure_l_abs_tol = 1e-10
  momentum_l_tol = 0
  pressure_l_tol = 0
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = 'timestep_end'
  []
  [pressure_drop]
    type = PressureDrop
    pressure = pressure
    upstream_boundary = 'left'
    downstream_boundary = 'right'
    boundary = 'left right'
  []
  [pressure_drop_deviation]
    type = ParsedPostprocessor
    expression = 'abs(pressure_drop - ${reference_pressure_drop})'
    pp_names = 'pressure_drop'
  []
[]

[Outputs]
  exodus = false
  csv = true
  print_linear_residuals = false
  print_nonlinear_residuals = false
[]
