mu = 1.1
rho = 1.1
advected_interp_method = 'average'
velocity_interp_method = 'rc'

pressure_tag = "pressure_grad"

restricted_blocks = '1'

[Mesh]
  parallel_type = 'replicated'
  [mesh]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1'
    dy = '1'
    ix = '7 7'
    iy = 10
    subdomain_id = '1 2'
  []
  [mid]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 2
    input = mesh
    new_boundary = 'middle'
  []
  [break_top]
    type = PatchSidesetGenerator
    boundary = 'top'
    n_patches = 2
    input = mid
  []
  [break_bottom]
    type = PatchSidesetGenerator
    boundary = 'bottom'
    n_patches = 2
    input = break_top
  []
[]

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[Problem]
  nl_sys_names = 'u_system v_system pressure_system energy_system scalar_system'
  previous_nl_solution_required = true
  kernel_coverage_check = false
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolatorSegregated
    u = vel_x
    v = vel_y
    pressure = pressure
    block = ${restricted_blocks}
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1.0
    solver_sys = u_system
    two_term_boundary_expansion = false
    block = ${restricted_blocks}
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0.0
    solver_sys = v_system
    two_term_boundary_expansion = false
    block = ${restricted_blocks}
  []
  [pressure]
    type = INSFVPressureVariable
    solver_sys = pressure_system
    initial_condition = 0.2
    two_term_boundary_expansion = false
    block = ${restricted_blocks}
  []
  [T_fluid]
    type = INSFVEnergyVariable
    initial_condition = 300
    solver_sys = energy_system
    two_term_boundary_expansion = false
    block = ${restricted_blocks}
  []
  [scalar]
    type = INSFVScalarFieldVariable
    block = ${restricted_blocks}
    solver_sys = scalar_system
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

  [energy_advection]
    type = INSFVEnergyAdvection
    variable = T_fluid
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    boundaries_to_force = 'bottom_0'
  []
  [energy_diffusion]
    type = FVDiffusion
    coeff = 1.1
    variable = T_fluid
  []
  [energy_loss]
    type = FVBodyForce
    variable = T_fluid
    value = -0.1
  []

  [scalar_advection]
    type = INSFVScalarFieldAdvection
    variable = scalar
    velocity_interp_method = ${velocity_interp_method}
    advected_interp_method = ${advected_interp_method}
    boundaries_to_force = 'bottom_0'
  []
  [scalar_diffusion]
    type = FVDiffusion
    coeff = 1.0
    variable = scalar
  []
  [scalar_src]
    type = FVBodyForce
    variable = scalar
    value = 0.1
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    function = '1.0'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    function = '0.0'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top_0'
    variable = vel_x
    function = 0.0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top_0'
    variable = vel_y
    function = 0.0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'middle'
    variable = pressure
    function = 0
  []
  [inlet_t]
    type = FVDirichletBC
    boundary = 'left'
    variable = T_fluid
    value = 1
  []
  [outlet_scalar]
    type = FVDirichletBC
    boundary = 'middle'
    variable = scalar
    value = 1
  []
  [symmetry-u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom_0'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = 'x'
  []
  [symmetry-v]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom_0'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [symmetry-p]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom_0'
    variable = pressure
  []
[]

[Executioner]
  type = SIMPLENonlinearAssembly
  momentum_l_abs_tol = 1e-12
  pressure_l_abs_tol = 1e-12
  energy_l_abs_tol = 1e-12
  passive_scalar_l_abs_tol = 1e-12
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  passive_scalar_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  passive_scalar_systems = 'scalar_system'
  pressure_gradient_tag = ${pressure_tag}
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 0.3
  energy_equation_relaxation = 0.99
  passive_scalar_equation_relaxation = 0.99
  num_iterations = 100
  pressure_absolute_tolerance = 1e-9
  momentum_absolute_tolerance = 1e-9
  energy_absolute_tolerance = 1e-9
  passive_scalar_absolute_tolerance = 1e-9
  print_fields = false
[]

[FunctorMaterials]
  [const_functor]
    type = ADGenericFunctorMaterial
    prop_names = 'cp'
    prop_values = '2'
  []
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    rho = ${rho}
    temperature = 'T_fluid'
    block = ${restricted_blocks}
  []
[]

[Outputs]
  exodus = true
  csv = false
  perf_graph = false
  print_nonlinear_residuals = false
  print_linear_residuals = true
[]
