# Coupled conduction and radiative heat transfer test in the linear FV system using SIMPLE solver.
# A 1D P1 radiation model is solved in the fluid region. Energy is solved in the
# fluid and solid region using two different variables. The radiative and conductive flux
# is maintained at the fluid-solid interface using conjugate heat transfer, RobinCHT for
# the solid side and Dirichlet CHT for the fluid side. Temperatures are set at the solid and
# fluid boundaries.
### Benchmark sources:
### https://hal.science/hal-02070285/document

nx = 10
Tw_left = 1.0
Tw_right = 0.0
sigma = 5.670374419e-8

sigma_a = 1.0

N = 0.1 # Stark number N = k sigma_a / (4*sigma*pow(T_w,3))
k_salt = ${fparse N*4*sigma*pow(Tw_left,3)/sigma_a}
k_solid = ${fparse 4.0*k_salt}
diffusion_coef = ${fparse 1/(3*sigma_a)}
b_eps = 1.0

rho = 1.
mu = 5.
h_s = 0.0

advected_interp_method = 'upwind'

[Mesh]
  [salt_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = ${nx}
    xmin = 0
    xmax = 1
    subdomain_ids = 0
  []
  [solid_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = ${nx}
    xmin = 1
    xmax = 2
    subdomain_ids = 1
  []
  [give_name_solid]
    type = RenameBlockGenerator
    input = solid_mesh
    old_block = 1
    new_block = 'solid'
  []
  [give_name_salt]
    type = RenameBlockGenerator
    input = salt_mesh
    old_block = 0
    new_block = 'salt'
  []
  [stitch]
    type = StitchMeshGenerator
    inputs = 'give_name_salt give_name_solid'
    stitch_boundaries_pairs = 'right left'
  []
  [interface]
    input = stitch
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 'solid'
    paired_block = 'salt'
    new_boundary = interface
  []
  # Prevent test diffing on distributed parallel element numbering
  allow_renumbering = false
[]

[Problem]
  linear_sys_names = 'u_system pressure_system energy_system solid_energy_system radiation_system '
  previous_nl_solution_required = true
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
    block = 'salt'
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 1e-6
    solver_sys = 'u_system'
    block = 'salt'
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = 'pressure_system'
    initial_condition = 1e-6
    block = 'salt'
  []
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = 'energy_system'
    initial_condition = ${fparse Tw_left/2}
    block = 'salt'
  []
  [G]
    type = MooseLinearVariableFVReal
    solver_sys = 'radiation_system'
    initial_condition = ${fparse 4*sigma*pow(Tw_left/2,4)}
    block = 'salt'
  []
  [T_solid]
    type = MooseLinearVariableFVReal
    solver_sys = 'solid_energy_system'
    initial_condition = ${fparse Tw_left/2}
    block = 'solid'
  []
[]

[Postprocessors]
  [T_fluid_iface]
    type = ElementalVariableValue
    variable = T_fluid
    elementid = ${fparse nx-1}
  []
  [T_solid_iface]
    type = ElementalVariableValue
    variable = T_solid
    elementid = ${fparse nx}
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    mu = ${mu}
    u = vel_x
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
  []

  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []

  [fluid_conduction]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = ${k_salt}
    use_nonorthogonal_correction = false
    block = salt
  []
  [fluid_radiation]
    type = LinearFVP1TemperatureSourceSink
    variable = T_fluid
    G = 'G'
    absorption_coeff = ${sigma_a}
    block = salt
  []

  [G_diffusion]
    type = LinearFVDiffusion
    variable = G
    diffusion_coeff = ${diffusion_coef}
    block = salt
  []
  [G_source_and_sink]
    type = LinearFVP1RadiationSourceSink
    variable = G
    temperature_radiation = 'T_fluid'
    absorption_coeff = ${sigma_a}
    block = salt
  []

  [solid_conduction]
    type = LinearFVDiffusion
    variable = T_solid
    diffusion_coeff = ${k_solid}
    use_nonorthogonal_correction = false
    block = solid
  []
[]

[LinearFVBCs]
  # velocity BCs
  [walls-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left interface'
    variable = vel_x
    functor = 0.0
  []

  # temperature BCs
  [left_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_fluid
    boundary = 'left'
    functor = ${Tw_left}
  []
  [right_bc_T]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T_solid
    boundary = 'right'
    functor = ${Tw_right}
  []

  # G
  [right_bc_G]
    type = LinearFVP1RadiationMarshakBC
    boundary = 'interface'
    variable = G
    temperature_radiation = 'T_fluid'
    coeff_diffusion = ${diffusion_coef}
    boundary_emissivity = ${b_eps}
  []
  [left_bc_G]
    type = LinearFVP1RadiationMarshakBC
    boundary = 'left'
    variable = G
    temperature_radiation = ${Tw_left}
    coeff_diffusion = ${diffusion_coef}
    boundary_emissivity = ${b_eps}
  []

  # cht
  [fluid_solid]
    type = LinearFVDirichletCHTBC
    variable = T_fluid
    boundary = interface
    functor = interface_temperature_solid_interface
  []
  [solid_fluid]
    type = LinearFVRobinCHTBC
    variable = T_solid
    boundary = interface
    h = ${h_s}
    thermal_conductivity = ${k_solid}
    incoming_flux = heat_flux_to_solid_interface
    surface_temperature = interface_temperature_fluid_interface
  []
[]

[Executioner]
  type = SIMPLE

  num_iterations = 2000

  momentum_systems = 'u_system '
  pressure_system = 'pressure_system'
  rhie_chow_user_object = 'rc'
  momentum_l_abs_tol = 1e-8
  pressure_l_abs_tol = 1e-7
  momentum_l_tol = 0
  pressure_l_tol = 0
  momentum_equation_relaxation = 0.7
  pressure_variable_relaxation = 0.3
  momentum_absolute_tolerance = 1e-5
  pressure_absolute_tolerance = 1e-5
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'

  #active_scalar_systems = 'radiation_system'
  pm_radiation_systems = 'radiation_system'
  energy_system = 'energy_system'
  solid_energy_system = 'solid_energy_system'
  #active_scalar_l_abs_tol = 1e-14
  pm_radiation_l_abs_tol = 1e-11
  energy_l_abs_tol = 1e-11
  solid_energy_l_abs_tol = 1e-11
  #active_scalar_l_tol = 0
  pm_radiation_l_tol = 0
  energy_l_tol = 0
  solid_energy_l_tol = 0
  #active_scalar_equation_relaxation = 0.95
  pm_radiation_equation_relaxation = 1.0
  energy_equation_relaxation = 0.9
  energy_absolute_tolerance = 1e-10
  solid_energy_absolute_tolerance = 1e-10
  #active_scalar_absolute_tolerance = 1e-14
  #active_scalar_petsc_options_iname = '-pc_type -pc_hypre_type'
  #active_scalar_petsc_options_value = 'hypre boomeramg'
  pm_radiation_absolute_tolerance = 1e-10
  pm_radiation_petsc_options_iname = '-pc_type -pc_hypre_type'
  pm_radiation_petsc_options_value = 'hypre boomeramg'

  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  solid_energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  solid_energy_petsc_options_value = 'hypre boomeramg'

  cht_interfaces = 'interface'
  cht_solid_flux_relaxation = 0.3
  cht_fluid_flux_relaxation = 0.3
  cht_solid_temperature_relaxation = 0.3
  cht_fluid_temperature_relaxation = 0.3
  cht_heat_flux_tolerance = 1e-4
  max_cht_fpi = 10

  print_fields = false
  continue_on_max_its = true

[]

[Outputs]
  csv = true
  execute_on = timestep_end
  exodus = true
[]
