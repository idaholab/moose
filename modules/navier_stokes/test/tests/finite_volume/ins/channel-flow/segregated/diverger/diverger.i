mu = 2.6
rho = 1.0
cp = 700
advected_interp_method = 'upwind'
velocity_interp_method = 'rc'

pressure_tag = "pressure_grad"

[Mesh]
  # uniform_refine = 1
  [fmg]
    type = FileMeshGenerator
    file = "diverger-2d.msh"
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system energy_system'
  previous_nl_solution_required = true
  error_on_jacobian_nonzero_reallocation = true
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolatorSegregated
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0.5
    solver_sys = u_system
    two_term_boundary_expansion = false
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    solver_sys = v_system
    two_term_boundary_expansion = false
  []
  [pressure]
    type = INSFVPressureVariable
    solver_sys = pressure_system
    initial_condition = 0.2
    # two_term_boundary_expansion = false
  []
  [T]
    type = INSFVEnergyVariable
    two_term_boundary_expansion = false
    solver_sys = energy_system
    initial_condition = 700
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

  [heat_advection]
    type = INSFVEnergyAdvection
    variable = T
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
  []
  [heat_diffusion]
    type = FVDiffusion
    variable = T
    coeff = '10'
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = vel_x
    function = '1.1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'inlet'
    variable = vel_y
    function = '0.0'
  []
  [inlet-T]
    type = FVDirichletBC
    boundary = 'inlet'
    value = 700
    variable = T
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = vel_x
    function = 0.0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = vel_y
    function = 0.0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'outlet'
    variable = pressure
    function = 1.4
  []
  [zerograd-p]
    type = FVNeumannBC
    boundary = 'top bottom inlet'
    variable = pressure
    value = 0
  []
[]
[FunctorMaterials]
  [mu]
    type = ADGenericFunctorMaterial #defines mu artificially for numerical convergence
    prop_names = 'mu rho cp' #it converges to the real mu eventually.
    prop_values = '${mu} ${rho} ${cp}'
  []
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    rho = ${rho}
    cp = ${cp}
    temperature = 'T'
  []
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  energy_l_abs_tol = 1e-12
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  num_iterations = 100
  pressure_absolute_tolerance = 1e-13
  momentum_absolute_tolerance = 1e-13
  energy_absolute_tolerance = 1e-13
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
