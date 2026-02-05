[Problem]
  kernel_coverage_check = false
  linear_sys_names = 'energy_system u_system v_system p_system'
[]

[GlobalParams]
  block = '1 3'
[]

[Mesh]
  type = MeshGeneratorMesh

  [./cartesian]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1'
    ix = '10 8 10'
    dy = '5'
    iy = '30'
    subdomain_id = '1 2 3'
  [../]

  [./break_sides]
    type = BreakBoundaryOnSubdomainGenerator
    boundaries = 'bottom top'
    input = cartesian
  [../]

  [./left_interior]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 2
    new_boundary = left_interior
    input = break_sides
  [../]

  [./right_interior]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 3
    paired_block = 2
    new_boundary = right_interior
    input = left_interior
  [../]
  [./rename]
    type = RenameBlockGenerator
    input = right_interior
    old_block = '1 2 3'
    new_block = '1 4 3'
  [../]
  [delete_others]
    type = BoundaryDeletionGenerator
    input = 'rename'
    boundary_names = 'bottom top'
  []
  # [delete_block5]
  #   type = BlockDeletionGenerator
  #   input = 'delete_others'
  #   block = '4'
  # []
[]


[Variables]
  [temperature]
    type = MooseLinearVariableFVReal
    solver_sys = 'energy_system'
    initial_condition = 1000
    block = '1 3'
  []
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_system'
    initial_condition = 0
    block = '1 3'
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = 'v_system'
    initial_condition = 0
    block = '1 3'
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = 'p_system'
    initial_condition = 0
    block = '1 3'
  []
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = 1
    p_diffusion_kernel = p_diffusion
  []
[]

[LinearFVKernels]
  [temp_conduction]
    type = LinearFVDiffusion
    diffusion_coeff = 1
    variable = temperature
    block = '1 3'
  []

  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
    block = '1 3'
  []
[]

[UserObjects]
  [./cavity_radiation]
    type = ConstantViewFactorSurfaceRadiation
    boundary = 'left_interior right_interior bottom_to_2 top_to_2'
    temperature = temperature
    emissivity = '0.8 0.8 0.8 0.8'
    adiabatic_boundary = 'bottom_to_2 top_to_2'
    # these view factors are made up to exactly balance energy
    # transfer through the cavity
    view_factors = '0    0.8 0.1 0.1;
                    0.8  0   0.1 0.1;
                    0.45 0.45  0 0.1;
                    0.45 0.45 0.1  0'
    execute_on = 'INITIAL LINEAR TIMESTEP_END NONLINEAR'
  [../]
[]

[LinearFVBCs]
  [./bottom_left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = temperature
    boundary = bottom_to_1
    functor = 1500.
  [../]
  [./top_right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = temperature
    boundary = 'top_to_3'
    functor = 300.
  [../]
  [./radiation]
    type = LinearFVGrayLambert
    variable = temperature
    temperature_radiation = temperature
    coeff_diffusion = 1
    #reconstruct_emission = false
    surface_radiation_object_name = cavity_radiation
    boundary = 'left_interior right_interior'
  [../]
[]

[Postprocessors]
  [./qdot_left]
    type = GrayLambertSurfaceRadiationPP
    boundary = left_interior
    surface_radiation_object_name = cavity_radiation
    return_type = HEAT_FLUX_DENSITY
  [../]

  [./qdot_right]
    type = GrayLambertSurfaceRadiationPP
    boundary = right_interior
    surface_radiation_object_name = cavity_radiation
    return_type = HEAT_FLUX_DENSITY
  [../]

  [./qdot_top]
    type = GrayLambertSurfaceRadiationPP
    boundary = top_to_2
    surface_radiation_object_name = cavity_radiation
    return_type = HEAT_FLUX_DENSITY
  [../]

  [./qdot_bottom]
    type = GrayLambertSurfaceRadiationPP
    boundary = bottom_to_2
    surface_radiation_object_name = cavity_radiation
    return_type = HEAT_FLUX_DENSITY
  [../]
[]

[Executioner]
  type = SIMPLE
  num_iterations = 2
  should_solve_momentum = false
  should_solve_pressure = false
  energy_system = 'energy_system'
  energy_l_abs_tol = 1e-14
  energy_l_tol = 1e-14
  energy_equation_relaxation = 0.1
  energy_field_relaxation = 0.1
  energy_absolute_tolerance = 1e-14
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true

  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'p_system'

  # [Quadrature]
  #   side_order = SECOND
  # []
[]

[Outputs]
  exodus = true
[]
