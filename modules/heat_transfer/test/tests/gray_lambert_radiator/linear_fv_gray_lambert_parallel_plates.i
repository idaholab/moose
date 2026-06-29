# Two conducting slabs radiating across an empty gap.
#
#
# Geometry, not to scale:
#
#   x = 0        x = 0.1        x = 0.2        x = 0.3
#   T = 1000 K   left_rad  <-- empty gap -->  right_rad   T = 300 K
#   | Dirichlet | left solid | deleted block | right solid | Dirichlet |
#
# Analytic reference for this file:
#   k_left = k_right = 10 W/m/K
#   L_left = L_right = 0.1 m
#   eps_left = 0.8
#   eps_right = 0.6
#   F_left,right = F_right,left = 1
#   sigma = 5.67037e-8 W/m^2/K^4
#
#   Thermal Resistance network solution
#   q = sigma*[(T_left-q*L_left/k_left)^4 - (T_right-q*L_right/k_right)^4]/[1/eps_left + 1/eps_right -1]
#
#   Analytical Solutions:
#   q = 14581.2627500345 W/m^2
#   T_left_rad = 854.187372499655 K
#   T_right_rad = 445.812627500345 K

[Problem]
  kernel_coverage_check = false
  linear_sys_names = 'energy_system u_system v_system p_system'
[]

[Mesh]
  type = MeshGeneratorMesh

  [cmg]
    type = CartesianMeshGenerator
    dim = 2

    # Three columns: left solid, temporary gap block, right solid.
    # The gap block is deleted below so radiation occurs across an empty volume.
    dx = '0.1 0.1 0.1'
    ix = '640 1 640'
    dy = '1.0'
    iy = '20'
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

  # Dummy flow variables keep this input aligned with the SIMPLE/LinearFV
  # structure in the reference file. Momentum and pressure solves are disabled.
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

    # Ordering matters. The view-factor matrix below is in this boundary order.
    boundary = 'left_rad right_rad'

    # No fixed-temperature or adiabatic radiating walls in this case.
    # Both participating surfaces get their temperatures from the FV variable.
    fixed_temperature_boundary = ''
    fixed_boundary_temperatures = ''
    adiabatic_boundary = ''

    emissivity = '0.8 0.6'
    temperature = temperature

    # Prescribed two-surface exchange: F_12 = F_21 = 1, F_11 = F_22 = 0.
    # This removes view-factor validation from the test.
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

  # Present only to satisfy the RhieChowMassFlux user object used by SIMPLE.
  # Pressure is not solved in this validation case.
  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
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
    type = LinearFVGrayLambert
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

  # Gold-value error monitors.  These are convenient for CSV diffs or tests.
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
  num_iterations = 30000

  should_solve_momentum = false
  should_solve_pressure = false

  energy_system = 'energy_system'
  energy_l_abs_tol = 1e-14
  energy_l_tol = 1e-14
  energy_equation_relaxation = 0.99
  energy_field_relaxation = 0.99
  energy_absolute_tolerance = 1e-14
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'

  print_fields = false
  continue_on_max_its = true

  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'p_system'
[]

[Outputs]
  exodus = true
  csv = true
[]
