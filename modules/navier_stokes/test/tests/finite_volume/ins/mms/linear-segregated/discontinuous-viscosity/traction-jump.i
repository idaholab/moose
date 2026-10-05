rho = 1
mu_left = 1
mu_right = 4
eps_left = 1
eps_right = 1
interface = 0.5
skew = 0.35
normal_velocity = 1
tangential_velocity = 0
tangential_gradient = 0.4
tangential_curvature = 0
shear_traction = 2

# In interface-normal coordinates (r,t), the manufactured velocity is
# v_n = normal_velocity + tangential_gradient*t and
# v_t = tangential_velocity + (shear_traction/mu - tangential_gradient)*r
#       + tangential_curvature*r^2. It is divergence-free and
# mu*(d(v_t)/dr + d(v_n)/dt) = shear_traction on both sides of the viscosity jump.

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
  []
  [right]
    type = ParsedSubdomainMeshGenerator
    input = gen
    combinatorial_geometry = 'x > ${interface}'
    block_id = 1
    block_name = right
  []
  [left]
    type = ParsedSubdomainMeshGenerator
    input = right
    combinatorial_geometry = 'x < ${interface}'
    block_id = 2
    block_name = left
  []
  [skew]
    type = ParsedNodeTransformGenerator
    input = left
    x_function = 'x + ${skew} * y'
  []
[]

[UserObjects]
  [rc]
    type = PorousRhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = ${rho}
    porosity = porosity
    p_diffusion_kernel = p_diffusion
    pressure_projection_method = consistent
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = 0
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0
  []
[]

[FVInterpolationMethods]
  [average]
    type = FVGeometricAverage
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearPWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method_name = average
    mu = dynamic_viscosity
    u = vel_x
    v = vel_y
    momentum_component = x
    rhie_chow_user_object = rc
    use_two_point_stress_transmissibility = true
    use_nonorthogonal_correction = true
    use_deviatoric_terms = true
  []
  [v_advection_stress]
    type = LinearPWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method_name = average
    mu = dynamic_viscosity
    u = vel_x
    v = vel_y
    momentum_component = y
    rhie_chow_user_object = rc
    use_two_point_stress_transmissibility = true
    use_nonorthogonal_correction = true
    use_deviatoric_terms = true
  []
  [u_pressure]
    type = LinearPWCNSFVMomentumPressure
    variable = vel_x
    pressure = pressure
    porosity = porosity
    momentum_component = x
  []
  [v_pressure]
    type = LinearPWCNSFVMomentumPressure
    variable = vel_y
    pressure = pressure
    porosity = porosity
    momentum_component = y
  []
  [u_forcing]
    type = LinearFVSource
    variable = vel_x
    source_density = forcing_u
  []
  [v_forcing]
    type = LinearFVSource
    variable = vel_y
    source_density = forcing_v
  []
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = true
    use_nonorthogonal_correction_on_boundary = true
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
[]

[LinearFVBCs]
  [velocity_x]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = vel_x
    functor = exact_u
  []
  [velocity_y]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = vel_y
    functor = exact_v
  []
  [pressure]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'left right top bottom'
    variable = pressure
    use_two_term_expansion = true
  []
[]

[FunctorMaterials]
  [viscosity_left]
    type = GenericFunctorMaterial
    block = left
    prop_names = dynamic_viscosity
    prop_values = ${mu_left}
  []
  [viscosity_right]
    type = GenericFunctorMaterial
    block = right
    prop_names = dynamic_viscosity
    prop_values = ${mu_right}
  []
  [porosity_left]
    type = GenericFunctorMaterial
    block = left
    prop_names = porosity
    prop_values = ${eps_left}
  []
  [porosity_right]
    type = GenericFunctorMaterial
    block = right
    prop_names = porosity
    prop_values = ${eps_right}
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = '(vn+skew*vt)/sqrt(1+skew^2)'
    symbol_names = 'skew vn vt'
    symbol_values = '${skew} exact_normal_velocity exact_tangential_velocity'
  []
  [exact_v]
    type = ParsedFunction
    expression = '(-skew*vn+vt)/sqrt(1+skew^2)'
    symbol_names = 'skew vn vt'
    symbol_values = '${skew} exact_normal_velocity exact_tangential_velocity'
  []
  [exact_normal_velocity]
    type = ParsedFunction
    expression = 'U+b*(skew*x+y)/sqrt(1+skew^2)'
    symbol_names = 'skew U b'
    symbol_values = '${skew} ${normal_velocity} ${tangential_gradient}'
  []
  [exact_tangential_velocity]
    type = ParsedFunction
    expression = 'Vt+(q/if(x-skew*y<interface,mu_left,mu_right)-b)'
                 '*(x-skew*y-interface)/sqrt(1+skew^2)'
                 '+c*((x-skew*y-interface)/sqrt(1+skew^2))^2'
    symbol_names = 'interface skew mu_left mu_right q b Vt c'
    symbol_values = '${interface} ${skew} ${mu_left} ${mu_right} ${shear_traction} ${tangential_gradient} ${tangential_velocity} ${tangential_curvature}'
  []
  [tangential_velocity_derivative]
    type = ParsedFunction
    expression = 'q/mu-b+2*c*(x-skew*y-interface)/sqrt(1+skew^2)'
    symbol_names = 'interface skew mu q b c'
    symbol_values = '${interface} ${skew} viscosity ${shear_traction} ${tangential_gradient} ${tangential_curvature}'
  []
  [forcing_u]
    type = ParsedFunction
    expression = '(b*vt+skew*(dvtdr*vn-2*mu*c))/sqrt(1+skew^2)'
    symbol_names = 'skew mu vn vt dvtdr b c'
    symbol_values = '${skew} viscosity exact_normal_velocity exact_tangential_velocity tangential_velocity_derivative ${tangential_gradient} ${tangential_curvature}'
  []
  [forcing_v]
    type = ParsedFunction
    expression = '(-skew*b*vt+dvtdr*vn-2*mu*c)/sqrt(1+skew^2)'
    symbol_names = 'skew mu vn vt dvtdr b c'
    symbol_values = '${skew} viscosity exact_normal_velocity exact_tangential_velocity tangential_velocity_derivative ${tangential_gradient} ${tangential_curvature}'
  []
  [viscosity]
    type = ParsedFunction
    expression = 'if(x-skew*y<interface,mu_left,mu_right)'
    symbol_names = 'interface skew mu_left mu_right'
    symbol_values = '${interface} ${skew} ${mu_left} ${mu_right}'
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-11
  pressure_l_abs_tol = 1e-11
  momentum_l_max_its = 50
  pressure_l_max_its = 50
  momentum_l_tol = 0
  pressure_l_tol = 0
  rhie_chow_user_object = rc
  momentum_systems = 'u_system v_system'
  pressure_system = pressure_system
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.5
  num_iterations = 5000
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false

  pin_pressure = true
  pressure_pin_value = 0
  pressure_pin_point = '0.425 0.5 0'
[]

[Outputs]
  exodus = false
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    outputs = csv
    execute_on = FINAL
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact_u
    outputs = csv
    execute_on = FINAL
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact_v
    outputs = csv
    execute_on = FINAL
  []
  [L2p]
    type = ElementL2FunctorError
    approximate = pressure
    exact = 0
    outputs = csv
    execute_on = FINAL
  []
[]
