[Problem]
  kernel_coverage_check = false
  linear_sys_names = 'energy_system u_system v_system p_system'
[]

[Mesh]
  type = MeshGeneratorMesh

  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1.3 1.9'
    ix = '5 5 5'
    dy = '2 1.2 0.9'
    iy = '5 5 5'
    subdomain_id = '0 1 0
                    4 5 2
                    0 3 0'
  []

  [inner_bottom]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    primary_block = 1
    paired_block = 5
    new_boundary = 'inner_bottom'
  []

  [inner_left]
    type = SideSetsBetweenSubdomainsGenerator
    input = inner_bottom
    primary_block = 4
    paired_block = 5
    new_boundary = 'inner_left'
  []

  [inner_right]
    type = SideSetsBetweenSubdomainsGenerator
    input = inner_left
    primary_block = 2
    paired_block = 5
    new_boundary = 'inner_right'
  []

  [inner_top]
    type = SideSetsBetweenSubdomainsGenerator
    input = inner_right
    primary_block = 3
    paired_block = 5
    new_boundary = 'inner_top'
  []

  [rename]
    type = RenameBlockGenerator
    old_block = '1 2 3 4'
    new_block = '0 0 0 0'
    input = inner_top
  []

  [split_inner_bottom]
    type = PatchSidesetGenerator
    boundary = 4
    n_patches = 2
    partitioner = centroid
    centroid_partitioner_direction = x
    input = rename
  []

  [split_inner_left]
    type = PatchSidesetGenerator
    boundary = 5
    n_patches = 2
    partitioner = centroid
    centroid_partitioner_direction = y
    input = split_inner_bottom
  []

  [split_inner_right]
    type = PatchSidesetGenerator
    boundary = 6
    n_patches = 2
    partitioner = centroid
    centroid_partitioner_direction = y
    input = split_inner_left
  []

  [split_inner_top]
    type = PatchSidesetGenerator
    boundary = 7
    n_patches = 3
    partitioner = centroid
    centroid_partitioner_direction = x
    input = split_inner_right
  []

  [delete_others]
    type = BoundaryDeletionGenerator
    input = 'split_inner_top'
    boundary_names = 'inner_bottom inner_top inner_left inner_right'
  []

  [delete_block5]
    type = BlockDeletionGenerator
    input = 'delete_others'
    block = '5'
  []
[]

[Variables]
  [temperature]
    type = MooseLinearVariableFVReal
    solver_sys = 'energy_system'
    initial_condition = 400
  []
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_system'
    initial_condition = 0
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = 'v_system'
    initial_condition = 0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = 'p_system'
    initial_condition = 0
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
    diffusion_coeff = 5.
    variable = temperature
  []

  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
  []
[]

[UserObjects]
  [gray_lambert]
    type = ViewFactorObjectSurfaceRadiation
    boundary = 'inner_bottom_0 inner_bottom_1
                inner_left_0 inner_left_1
                inner_right_0 inner_right_1
                inner_top_0 inner_top_1 inner_top_2'
    fixed_temperature_boundary = 'inner_bottom_0 inner_bottom_1'
    fixed_boundary_temperatures = '1200 1200'
    adiabatic_boundary = 'inner_top_0 inner_top_1 inner_top_2'
    emissivity = '0.9 0.9
                  0.8 0.8
                  0.4 0.4
                  1 1 1'
    temperature = temperature
    view_factor_object_name = view_factor
    execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END NONLINEAR'
  []

  [view_factor]
    type = UnobstructedPlanarViewFactor
    boundary = 'inner_bottom_0 inner_bottom_1
                inner_left_0 inner_left_1
                inner_right_0 inner_right_1
                inner_top_0 inner_top_1 inner_top_2'
    normalize_view_factor = true
    execute_on = 'INITIAL'
  []
[]

[LinearFVBCs]
  [left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = temperature
    boundary = 'left'
    functor = 600.
  [.]
  [right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = temperature
    boundary = 'right'
    functor = 300.
  []

  [radiation]
    type = LinearFVGrayLambertBC
    variable = temperature
    temperature_radiation = temperature
    coeff_diffusion = 5.
    surface_radiation_object_name = gray_lambert
    boundary = 'inner_left_0 inner_left_1
                inner_right_0 inner_right_1'
  []
[]

[Executioner]
  type = SIMPLE
  num_iterations = 2000
  should_solve_momentum = false
  should_solve_pressure = false
  energy_system = 'energy_system'
  energy_l_abs_tol = 1e-11
  energy_l_tol = 1e-11
  energy_equation_relaxation = 0.8
  energy_field_relaxation = 0.8
  energy_absolute_tolerance = 1e-10
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true

  [Quadrature]
    side_order = SECOND
  []

  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'p_system'
[]

[Outputs]
  exodus = true
[]
