[Problem]
  kernel_coverage_check = false
  linear_sys_names = 'energy_system'
[]

[Mesh]
  type = MeshGeneratorMesh

  [./cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1.3 1.9'
    ix = '3 3 3'
    dy = '2 1.2 0.9'
    iy = '3 3 3'
    subdomain_id = '0 1 0
                    4 5 2
                    0 3 0'
  [../]

  [./inner_bottom]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    primary_block = 1
    paired_block = 5
    new_boundary = 'inner_bottom'
  [../]

  [./inner_left]
    type = SideSetsBetweenSubdomainsGenerator
    input = inner_bottom
    primary_block = 4
    paired_block = 5
    new_boundary = 'inner_left'
  [../]

  [./inner_right]
    type = SideSetsBetweenSubdomainsGenerator
    input = inner_left
    primary_block = 2
    paired_block = 5
    new_boundary = 'inner_right'
  [../]

  [./inner_top]
    type = SideSetsBetweenSubdomainsGenerator
    input = inner_right
    primary_block = 3
    paired_block = 5
    new_boundary = 'inner_top'
  [../]

  [./rename]
    type = RenameBlockGenerator
    old_block = '1 2 3 4'
    new_block = '0 0 0 0'
    input = inner_top
  [../]

  [./split_inner_bottom]
    type = PatchSidesetGenerator
    boundary = 4
    n_patches = 2
    partitioner = centroid
    centroid_partitioner_direction = x
    input = rename
  [../]

  [./split_inner_left]
    type = PatchSidesetGenerator
    boundary = 5
    n_patches = 2
    partitioner = centroid
    centroid_partitioner_direction = y
    input = split_inner_bottom
  [../]

  [./split_inner_right]
    type = PatchSidesetGenerator
    boundary = 6
    n_patches = 2
    partitioner = centroid
    centroid_partitioner_direction = y
    input = split_inner_left
  [../]

  [./split_inner_top]
    type = PatchSidesetGenerator
    boundary = 7
    n_patches = 3
    partitioner = centroid
    centroid_partitioner_direction = x
    input = split_inner_right
  [../]

  [delete_others]
    type = BoundaryDeletionGenerator
    input = 'split_inner_top'
    boundary_names = 'inner_bottom inner_top inner_left inner_right'
  []
[]

[Variables]
  [temperature]
    type = MooseLinearVariableFVReal
    solver_sys = 'energy_system'
    initial_condition = 1200
    block = 0
  []
[]

[LinearFVKernels]
  [temp_conduction]
    type = LinearFVDiffusion
    diffusion_coeff = 5.
    block = 0
    variable = temperature
  []
[]

[UserObjects]
  [./gray_lambert]
    type = ViewFactorObjectSurfaceRadiation
    boundary = 'inner_bottom_0 inner_bottom_1
                inner_left_0 inner_left_1
                inner_right_0 inner_right_1
                inner_top_0 inner_top_1 inner_top_2'
    fixed_temperature_boundary = 'inner_bottom_0 inner_bottom_1'
    fixed_boundary_temperatures = '1200          1200'
    adiabatic_boundary = 'inner_top_0 inner_top_1 inner_top_2'
    emissivity = '0.9 0.9
                  0.8 0.8
                  0.4 0.4
                  1 1 1'
    temperature = temperature
    view_factor_object_name = view_factor
    execute_on = 'LINEAR TIMESTEP_END'
  [../]

  [./view_factor]
    type = UnobstructedPlanarViewFactor
    boundary = 'inner_bottom_0 inner_bottom_1
                inner_left_0 inner_left_1
                inner_right_0 inner_right_1
                inner_top_0 inner_top_1 inner_top_2'
    normalize_view_factor = true
    execute_on = 'INITIAL'
  [../]
[]

[LinearFVBCs]
  [./left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = temperature
    boundary = 'left'
    functor = 600.
  [../]
  [./right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = temperature
    boundary = 'right'
    functor = 300.
  [../]

  [./radiation]
    type = LinearFVGrayLambert
    variable = temperature
    temperature_radiation = temperature
    coeff_diffusion = 5.
    surface_radiation_object_name = gray_lambert
    boundary = 'inner_left_0 inner_left_1
                inner_right_0 inner_right_1'
  [../]

  [radiation_field]
    type = LinearFVGrayLambert
    variable = temperature
    temperature_radiation = 1200
    coeff_diffusion = 5.
    surface_radiation_object_name = gray_lambert
    boundary = 'inner_bottom_0 inner_bottom_1'
  []
[]

# [Postprocessors]
#   [./average_T_inner_right]
#     type = SideAverageValue
#     variable = temperature
#     boundary = inner_right
#   [../]
# []

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-energy_system_pc_type -energy_system_pc_factor_shift_type -snes_linesearch_damping'
  petsc_options_value = 'hypre boomeramg 0.8'
  l_abs_tol = 1e-20
  l_tol = 1e-20
  nl_abs_tol = 1e-14
  nl_forced_its = 10
  multi_system_fixed_point=true
  multi_system_fixed_point_convergence=linear
[]
[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 70
    converge_at_max_iterations = true
  []
[]
[Outputs]
  exodus = true
[]
