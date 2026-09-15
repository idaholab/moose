rho_salt = 2000
cp_salt = 2000
k_salt = 0.6

rho_solid = 2000
cp_solid = 900
k_solid = 10

T_hot = 400
T_cold = 300

R_interface = 0.1

L_solid = 0.5
L_salt = 0.5
nx_solid = 80
nx_salt = 80

# Exact 1D steady conduction solution for comparison.
q_exact = ${fparse (T_hot - T_cold) / (L_solid / k_solid + R_interface + L_salt / k_salt)}
T_solid_interface_exact = ${fparse T_cold + q_exact * L_solid / k_solid}
T_salt_interface_exact = ${fparse T_solid_interface_exact + q_exact * R_interface}

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
  previous_nl_solution_required = true
  linear_sys_names = 'energy_system  p_system u_system solid_energy_system'
[]

[FVInterpolationMethods]
  [harm]
    type = FVHarmonicAverage
  []
[]

[Mesh]
  [solid_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = ${nx_solid}
    xmin = 0.0
    xmax = ${L_solid}
    subdomain_ids = 0
    bias_x = 1.0
  []

  [fluid_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = ${nx_salt}
    xmin = ${L_solid}
    xmax = ${fparse L_solid + L_salt}
    subdomain_ids = 1
    bias_x = 1.0
  []

  [name_solid]
    type = RenameBlockGenerator
    input = solid_mesh
    old_block = 0
    new_block = 'solid'
  []

  [name_salt]
    type = RenameBlockGenerator
    input = fluid_mesh
    old_block = 1
    new_block = 'salt'
  []

  [stitch]
    type = StitchMeshGenerator
    inputs = 'name_solid name_salt'
    stitch_boundaries_pairs = 'right left'
  []

  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = stitch
    primary_block = 'solid'
    paired_block = 'salt'
    new_boundary = interface
  []
[]

[Functions]
  [T_solid_exact]
    type = ParsedFunction
    expression = '${T_cold} + ${q_exact} * x / ${k_solid}'
  []

  [T_salt_exact]
    type = ParsedFunction
    expression = '${T_salt_interface_exact} + ${q_exact} * (x - ${L_solid}) / ${k_salt}'
  []
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    w = vel_z
    pressure = pressure
    rho = 1
    p_diffusion_kernel = p_diffusion
    block = salt
  []
[]

[Variables]
  [temp_salt]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    block = salt
  []

  [temp_solid]
    type = MooseLinearVariableFVReal
    solver_sys = solid_energy_system
    block = solid
  []

  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_system'
    initial_condition = 0
    block = salt
  []

  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = 'p_system'
    initial_condition = 0
    block = salt
  []
[]

[FVICs]
  [solid_exact_ic]
    type = FVFunctionIC
    variable = temp_solid
    function = T_solid_exact
    block = solid
  []

  [salt_exact_ic]
    type = FVFunctionIC
    variable = temp_salt
    function = T_salt_exact
    block = salt
  []
[]

[LinearFVKernels]
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    block = salt
    use_nonorthogonal_correction = false
  []

  [h_time]
    type = LinearFVTimeDerivative
    variable = temp_salt
    factor = ${fparse rho_salt * cp_salt}
    block = salt
  []

  [h_conduction]
    type = LinearFVDiffusion
    variable = temp_salt
    diffusion_coeff = ${k_salt}
    use_nonorthogonal_correction = false
    block = salt
    coeff_interp_method = harm
  []

  [solid_time]
    type = LinearFVTimeDerivative
    variable = temp_solid
    factor = ${fparse rho_solid * cp_solid}
    block = solid
  []

  [solid_conduction]
    type = LinearFVDiffusion
    variable = temp_solid
    diffusion_coeff = ${k_solid}
    use_nonorthogonal_correction = false
    block = solid
    coeff_interp_method = harm
  []
[]

[FunctorMaterials]
[]

[LinearFVBCs]
  [solid_left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = temp_solid
    boundary = left
    functor = ${T_cold}
  []

  [fluid_right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = temp_salt
    boundary = right
    functor = ${T_hot}
  []

  # Neumann-Dirichlet CHT pair. The Robin BC emulates a Neumann BC because h = 0.
  # This avoids adding an unrelated heat-transfer coefficient to the accuracy test.
  [fluid_solid]
    type = LinearFVRobinCHTBC
    variable = temp_salt
    boundary = interface
    h = 0
    thermal_conductivity = ${k_salt}
    incoming_flux = heat_flux_to_fluid_interface
    surface_temperature = interface_temperature_to_fluid_interface
  []

  [solid_fluid]
    type = LinearFVDirichletCHTBC
    variable = temp_solid
    boundary = interface
    functor = interface_temperature_to_solid_interface
  []
[]

[Postprocessors]
  # Numerical interface quantities
  [q]
    type = SideAverageFunctorPostprocessor
    boundary = interface
    functor = heat_flux_to_solid_interface
    functor_argument = face
  []

  [t_solid]
    type = SideAverageFunctorPostprocessor
    boundary = interface
    functor = interface_temperature_to_solid_interface
    functor_argument = face
  []

  [t_fluid]
    type = SideAverageFunctorPostprocessor
    boundary = interface
    functor = interface_temperature_to_fluid_interface
    functor_argument = face
  []

  # Analytical interface quantities
  [q_analytical]
    type = ParsedPostprocessor
    expression = '${q_exact}'
  []

  [t_solid_analytical]
    type = ParsedPostprocessor
    expression = '${T_solid_interface_exact}'
  []

  [t_fluid_analytical]
    type = ParsedPostprocessor
    expression = '${T_salt_interface_exact}'
  []

  [q_relative_error]
    type = ParsedPostprocessor
    pp_names = 'q q_analytical'
    expression = 'abs(q-q_analytical)/abs(q_analytical)'
  []

  [t_solid_relative_error]
    type = ParsedPostprocessor
    pp_names = 't_solid t_solid_analytical'
    expression = 'abs(t_solid-t_solid_analytical)/abs(t_solid_analytical)'
  []

  [t_fluid_relative_error]
    type = ParsedPostprocessor
    pp_names = 't_fluid t_fluid_analytical'
    expression = 'abs(t_fluid-t_fluid_analytical)/abs(t_fluid_analytical)'
  []
[]

[Executioner]
  type = PIMPLE

  # Large dt makes this effectively a steady-state conduction test in one step.
  dt = 1e6
  end_time = 1e6
  num_iterations = 2
  continue_on_max_its = true
  print_fields = false

  energy_system = energy_system
  solid_energy_system = solid_energy_system

  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system'
  pressure_system = 'p_system'
  should_solve_momentum = false
  should_solve_pressure = false

  energy_l_abs_tol = 1e-18
  energy_l_tol = 1e-18
  solid_energy_l_abs_tol = 1e-18
  solid_energy_l_tol = 1e-18

  energy_absolute_tolerance = 1e-18
  solid_energy_absolute_tolerance = 1e-18

  energy_equation_relaxation = 1.0
  # energy_field_relaxation = 1.0

  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  solid_energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  solid_energy_petsc_options_value = 'hypre boomeramg'

  cht_interfaces = interface
  thermal_resistance = ${R_interface}

  cht_solid_flux_relaxation = 0.2
  cht_fluid_flux_relaxation = 0.2
  cht_solid_temperature_relaxation = 0.2
  cht_fluid_temperature_relaxation = 0.2
  cht_heat_flux_tolerance = 1e-8
  max_cht_fpi = 50
[]

[Outputs]
  console = true
  [validation]
    type = CSV
    execute_on = TIMESTEP_END
    show = 'q q_analytical q_relative_error
            t_solid t_solid_analytical t_solid_relative_error
            t_fluid t_fluid_analytical t_fluid_relative_error'
    precision = 14
    scientific_notation = true
  []
[]
