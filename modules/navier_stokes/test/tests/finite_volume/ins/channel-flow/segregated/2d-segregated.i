mu = 2.6
rho = 1.0
advected_interp_method = 'average'
velocity_interp_method = 'rc'

momentum_tag = "non_pressure"

[Mesh]
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.3'
    dy = '0.3'
    ix = '3'
    iy = '3'
    subdomain_id = '1'
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Problem]
  nl_sys_names = 'momentum_system pressure_system'
  previous_nl_solution_required = true
  error_on_jacobian_nonzero_reallocation = true
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolatorSegregated
    u = u
    v = v
    pressure = pressure
  []
[]

[Variables]
  [u]
    type = INSFVVelocityVariable
    initial_condition = 0.5
    nl_sys = momentum_system
    two_term_boundary_expansion = false
  []
  [v]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    nl_sys = momentum_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    nl_sys = pressure_system
    initial_condition = 0.2
    two_term_boundary_expansion = false
  []
[]

[FVKernels]
  [u_advection]
    type = INSFVMomentumAdvection
    variable = u
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
    linearize = true
    extra_vector_tags = ${momentum_tag}
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = u
    mu = ${mu}
    momentum_component = 'x'
    extra_vector_tags = ${momentum_tag}
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = u
    momentum_component = 'x'
    pressure = pressure
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = v
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
    linearize = true
    extra_vector_tags = ${momentum_tag}
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = v
    mu = ${mu}
    momentum_component = 'y'
    extra_vector_tags = ${momentum_tag}
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = v
    momentum_component = 'y'
    pressure = pressure
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
    boundary = 'left'
    variable = u
    function = '1.1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = v
    function = '0.0'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = u
    function = 0.0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = v
    function = 0.0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = 1.4
  []
  [zero-grad-pressure]
    type = FVFunctionNeumannBC
    variable = pressure
    boundary = 'top left bottom'
    function = 0.0
  []
[]

[Executioner]
  type = SIMPLE
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type -pc_factor_shift_type'
  petsc_options_value = 'hypre boomeramg NONZERO'
  petsc_options = '-ksp_monitor'
  nl_max_its = 1
  l_max_its = 200
  l_abs_tol = 1e-8
  l_tol = 1e-8
  line_search = 'none'
  rhie_chow_user_object = 'rc'
  momentum_system = 'momentum_system'
  pressure_system = 'pressure_system'
  momentum_tag = ${momentum_tag}
  momentum_variable_relaxation = 0.96
  pressure_variable_relaxation = 0.2
  num_iterations = 100
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-5

[]

[Postprocessors]
  [inlet_p]
    type = SideAverageValue
    variable = 'pressure'
    boundary = 'left'
  []
  [outlet-u]
    type = SideIntegralVariablePostprocessor
    variable = u
    boundary = 'right'
  []
[]

[Outputs]
  exodus = false
  csv = false
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]
