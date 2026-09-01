# Two conducting slabs radiating across an empty gap.
#
# Analytic reference:
#   q = 14581.2627500345 W/m^2
#   T_left_rad = 854.187372499655 K
#   T_right_rad = 445.812627500345 K
#
# The mesh is deliberately smaller than the exploratory 640 x 20 mesh. The
# solution is one-dimensional, so this keeps the regression test inexpensive
# without changing the reference solution.

[Problem]
  kernel_coverage_check = false
  linear_sys_names = 'energy_system u_system v_system p_system'
[]

[Mesh]
  type = MeshGeneratorMesh

  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '0.1 0.1 0.1'
    ix = '20 1 20'
    dy = '1.0'
    iy = '1'
    subdomain_id = '1 5 2'
  []

  [left_rad_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = cmg
    primary_block = 1
    paired_block = 5
    new_boundary = 'left_rad'
  []

  [right_rad_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = left_rad_sideset
    primary_block = 2
    paired_block = 5
    new_boundary = 'right_rad'
  []

  [delete_gap]
    type = BlockDeletionGenerator
    input = right_rad_sideset
    block = '5'
  []

  [rename_solid_blocks]
    type = RenameBlockGenerator
    input = delete_gap
    old_block = '1 2'
    new_block = 'left_solid right_solid'
  []
[]

[Variables]
  [temperature]
    type = MooseLinearVariableFVReal
    solver_sys = 'energy_system'
    initial_condition = 600
  []

  # Dummy flow variables retain the SIMPLE/LinearFV setup. Their systems are
  # not solved in this validation problem.
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

  [gray_lambert]
    type = ConstantViewFactorSurfaceRadiation
    boundary = 'left_rad right_rad'
    emissivity = '0.8 0.6'
    temperature = temperature
    stefan_boltzmann_constant = 5.67037e-8
    view_factors = '0 1;
                    1 0'
    execute_on = 'LINEAR TIMESTEP_BEGIN TIMESTEP_END NONLINEAR'
  []
[]

[LinearFVKernels]
  [temp_conduction]
    type = LinearFVDiffusion
    variable = temperature
    diffusion_coeff = 10.
    block = 'left_solid right_solid'
    use_nonorthogonal_correction = false
  []

  # This object is required by RhieChowMassFlux, although pressure is disabled.
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    block = 'left_solid right_solid'
    use_nonorthogonal_correction = false
  []
[]

[LinearFVBCs]
  [hot_left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = temperature
    boundary = 'left'
    functor = 1000.
  []

  [cold_right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = temperature
    boundary = 'right'
    functor = 300.
  []

  [radiation]
    type = LinearFVGrayLambertBC
    variable = temperature
    temperature_radiation = temperature
    coeff_diffusion = 10.
    surface_radiation_object_name = gray_lambert
    boundary = 'left_rad right_rad'
  []
[]

[Postprocessors]
  [T_left_rad]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = TEMPERATURE
    boundary = left_rad
  []

  [T_right_rad]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = TEMPERATURE
    boundary = right_rad
  []

  [q_left_rad]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = HEAT_FLUX_DENSITY
    boundary = left_rad
  []

  [q_right_rad]
    type = GrayLambertSurfaceRadiationPP
    surface_radiation_object_name = gray_lambert
    return_type = HEAT_FLUX_DENSITY
    boundary = right_rad
  []

  [T_left_error]
    type = ParsedPostprocessor
    pp_names = 'T_left_rad'
    expression = 'abs(T_left_rad - 854.187372499655)/854.187372499655'
  []

  [T_right_error]
    type = ParsedPostprocessor
    pp_names = 'T_right_rad'
    expression = 'abs(T_right_rad - 445.812627500345)/445.812627500345'
  []

  [q_left_rel_error]
    type = ParsedPostprocessor
    pp_names = 'q_left_rad'
    expression = 'abs(abs(q_left_rad) - 14581.2627500345)/14581.2627500345'
  []

  [q_right_rel_error]
    type = ParsedPostprocessor
    pp_names = 'q_right_rad'
    expression = 'abs(abs(q_right_rad) - 14581.2627500345)/14581.2627500345'
  []
[]

[Executioner]
  type = SIMPLE
  num_iterations = 1000

  should_solve_momentum = false
  should_solve_pressure = false

  energy_system = 'energy_system'
  energy_l_abs_tol = 1e-14
  energy_l_tol = 1e-14
  energy_equation_relaxation = 0.99
  energy_field_relaxation = 0.99
  energy_absolute_tolerance = 1e-14
  energy_petsc_options_iname = '-pc_type'
  energy_petsc_options_value = 'lu'

  print_fields = false
  continue_on_max_its = false

  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'p_system'
[]

[Outputs]
  execute_on = FINAL
  exodus = true
  csv = true
[]
