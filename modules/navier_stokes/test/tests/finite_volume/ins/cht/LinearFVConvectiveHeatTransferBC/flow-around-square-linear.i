mu = 0.01
rho = 1.1
k = 0.0005
cp = 10
k_s = 3.0
h_conv = 5

power_density = 10000

advected_interp_method = 'upwind'

[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmin = 0
    ymin = 0
    ymax = 0.1
    xmax = 0.1
  []
  [subdomain1]
    type = SubdomainBoundingBoxGenerator
    input = generated_mesh
    block_name = subdomain1
    bottom_left = '0.04 0.04 0'
    block_id = 1
    top_right = '0.06 0.06 0'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = 0
    paired_block = 1
    new_boundary = interface
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system pressure_system energy_system solid_energy_system'
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
    block = 0
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 0.1
    solver_sys = u_system
    block = 0
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.01
    block = 0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0.2
    block = 0
  []
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    initial_condition = 300
    block = 0
  []
  [T_solid]
    type = MooseLinearVariableFVReal
    solver_sys = solid_energy_system
    initial_condition = 500
    block = 1
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = true
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = true
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
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = true
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []

  [h_advection]
    type = LinearFVEnergyAdvection
    variable = T_fluid
    advected_quantity = temperature
    cp = ${cp}
    advected_interp_method = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
  []
  [conduction]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = ${k}
    use_nonorthogonal_correction = true
  []

  [solid-conduction]
    type = LinearFVDiffusion
    variable = T_solid
    diffusion_coeff = ${k_s}
    use_nonorthogonal_correction = true
  []
  [solid-source]
    type = LinearFVSource
    variable = T_solid
    source_density = ${power_density}
  []
[]

[LinearFVBCs]
  [inlet-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_x
    functor = '0.1'
  []
  [inlet-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = vel_y
    functor = '0.0'
  []
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top bottom interface'
    variable = vel_x
    functor = 0.0
  []
  [walls-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'top bottom interface'
    variable = vel_y
    functor = 0.0
  []
  [outlet_p]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'right'
    variable = pressure
    functor = 1.4
  []
  [outlet_u]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_x
    use_two_term_expansion = false
    boundary = right
  []
  [outlet_v]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = vel_y
    use_two_term_expansion = false
    boundary = right
  []

  [inlet_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    functor = 300
    boundary = 'left'
  []
  [walls_T]
    type = LinearFVAdvectionDiffusionFunctorNeumannBC
    variable = T_fluid
    functor = 0.0
    boundary = 'top bottom'
  []
  [outlet_T]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = T_fluid
    use_two_term_expansion = false
    boundary = right
  []

  [fluid_solid]
    type = LinearFVConvectiveHeatTransferBC
    variable = T_fluid
    T_solid = T_solid
    T_fluid = T_fluid
    boundary = interface
    h = ${h_conv}
  []
  [solid_fluid]
    type = LinearFVConvectiveHeatTransferBC
    variable = T_solid
    T_solid = T_solid
    T_fluid = T_fluid
    boundary = interface
    h = ${h_conv}
  []
[]

[FunctorMaterials]
  [rhocpT]
    property_name = 'rhocpT'
    type = ParsedFunctorMaterial
    functor_names = 'T_fluid'
    expression = '${rho}*${cp}*T_fluid'
  []
[]

[Postprocessors]
  [h_in]
    type = VolumetricFlowRate
    boundary = left
    vel_x = vel_x
    vel_y = vel_y
    rhie_chow_user_object = rc
    advected_quantity = 'rhocpT'
    subtract_mesh_velocity = false
  []
  [h_out]
    type = VolumetricFlowRate
    boundary = right
    vel_x = vel_x
    vel_y = vel_y
    rhie_chow_user_object = rc
    advected_quantity = 'rhocpT'
    advected_interp_method = upwind
    subtract_mesh_velocity = false
  []
  [power]
    type = ElementIntegralFunctorPostprocessor
    functor = ${power_density}
    block = 1
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  energy_l_abs_tol = 1e-13
  solid_energy_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  energy_l_tol = 0
  solid_energy_l_tol = 0
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  energy_system = 'energy_system'
  solid_energy_system = 'solid_energy_system'
  momentum_equation_relaxation = 0.8
  energy_equation_relaxation = 1.0
  pressure_variable_relaxation = 0.3
  num_iterations = 1000
  pressure_absolute_tolerance = 1e-10
  momentum_absolute_tolerance = 1e-10
  energy_absolute_tolerance = 1e-10
  solid_energy_absolute_tolerance = 1e-10
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  solid_energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  solid_energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
