mu = 0.002
rho = 1.0
advected_interp_method = 'average'
velocity_interp_method = 'rc'

pressure_tag = "pressure_grad"

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 3
    dx = '0.2'
    dy = '0.2'
    dz = '0.8'
    ix = '3'
    iy = '3'
    iz = '12'
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Problem]
  nl_sys_names = 'u_system v_system w_system pressure_system'
  previous_nl_solution_required = true
  error_on_jacobian_nonzero_reallocation = true
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolatorSegregated
    u = vel_x
    v = vel_y
    w = vel_z
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    solver_sys = u_system
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    solver_sys = v_system
    two_term_boundary_expansion = false
  []
  [vel_z]
    type = INSFVVelocityVariable
    initial_condition = 0.5
    solver_sys = w_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    solver_sys = pressure_system
    initial_condition = 0.2
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []
  [w_advection]
    type = INSFVMomentumAdvection
    variable = vel_z
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [w_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_z
    mu = ${mu}
    momentum_component = 'z'
  []
  [w_pressure]
    type = INSFVMomentumPressure
    variable = vel_z
    momentum_component = 'z'
    pressure = pressure
    extra_vector_tags = ${pressure_tag}
  []
  [p_diffusion]
    type = FVAnisotropicDiffusion
    variable = pressure
    coeff = "Ainv"
    coeff_interp_method = 'average'
  []
  [p_source]
    type = FVDivergence
    variable = pressure
    vector_field = "HbyA"
    force_boundary_execution = true
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'back'
    variable = vel_x
    function = '0'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'back'
    variable = vel_y
    function = '0'
  []
  [inlet-w]
    type = INSFVInletVelocityBC
    boundary = 'back'
    variable = vel_z
    function = '1.1'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'left right top bottom '
    variable = vel_x
    function = 0.0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'left right top bottom'
    variable = vel_y
    function = 0.0
  []
  [walls-w]
    type = INSFVNoSlipWallBC
    boundary = 'left right top bottom'
    variable = vel_z
    function = 0.0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'front'
    variable = pressure
    function = 1.4
  []
  [zero-grad-pressure]
    type = FVFunctionNeumannBC
    variable = pressure
    boundary = 'back left right top bottom'
    function = 0.0
  []
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system w_system'
  pressure_system = 'pressure_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.6
  pressure_variable_relaxation = 0.3
  num_iterations = 100
  pressure_absolute_tolerance = 1e-13
  momentum_absolute_tolerance = 1e-13

  momentum_l_abs_tol = 1e-14
  pressure_l_abs_tol = 1e-14
  momentum_l_tol = 0
  pressure_l_tol = 0
  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]
