rho_salt = 1
cp_salt = 1
k_salt = 1
L = 1

# Numerical regularization of sharp melting at T_m = 0.
# For a sharper run, reduce this to 1e-4 or 1e-5.
# Setting it exactly to zero activates your degenerate sharp branch.
T_solidus = 0
T_liquidus = 1e-2
T_hot_bc = 1

erf_lambda = 0.6194595791366345
t0 = 0.0065023282

simulation_end_time = 0.25

Lx = 2.0
nx = 1000

[Problem]
  previous_nl_solution_required = true
  linear_sys_names = 'energy_system p_system u_system'
[]

[Mesh]
  [salt_mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = ${nx}
    xmin = 0.0
    xmax = ${Lx}
    subdomain_ids = 0
    bias_x = 1.0
  []

  [name_salt]
    type = RenameBlockGenerator
    input = salt_mesh
    old_block = 0
    new_block = 'salt'
  []
[]

[Functions]
  # Initial temperature corresponding to the sharp analytical solution.
  [T_exact_initial]
    type = ParsedFunction
    expression = 'if(x < 0.1, 1.0 - erf(x / (2.0 * sqrt(${t0}))) / ${erf_lambda}, 0.0)'
  []
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    w = vel_z
    pressure = pressure
    rho = ${rho_salt}
    p_diffusion_kernel = p_diffusion
    block = salt
  []
[]

[Variables]
  [T]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
    block = salt
  []

  # Dummy velocity/pressure variables kept because your PIMPLE block
  # references u_system and p_system, even though they are not solved.
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

[AuxVariables]
  [fl]
    type = MooseVariableFVReal
    initial_condition = 0.0
  []
[]

[AuxKernels]
  [compute_fl]
    type = NSLiquidFractionAux
    variable = fl
    temperature = T
    T_liquidus = '${T_liquidus}'
    T_solidus = '${T_solidus}'
    execute_on = 'TIMESTEP_END'
  []
[]

[ICs]
  [h_salt_ic]
    type = FunctionIC
    variable = T
    function = T_exact_initial
    block = salt
  []
[]

[LinearFVKernels]
  # Pressure diffusion kernel kept only because the Rhie-Chow object
  # references it. Pressure solve is disabled in the Executioner.
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    block = salt
    use_nonorthogonal_correction = false
  []

  # rho * dh/dt
  [h_time]
    type = LinearFVTimeDerivative
    variable = T
    factor = ${fparse rho_salt*cp_salt}
    block = salt
  []

  # div(k * dT/dh * grad(h))
  [h_conduction]
    type = LinearFVDiffusion
    variable = T
    diffusion_coeff = ${k_salt}
    use_nonorthogonal_correction = false
    block = salt
    # coeff_interp_method = harm
  []
  [energy_source]
    type = LinearFVPhaseChangeSource
    variable = T
    L = ${L}
    T_liquidus = ${T_liquidus}
    T_solidus = ${T_solidus}
    rho = ${rho_salt}
    smoothing = 'sharp'
  []
[]

[LinearFVBCs]
  # Hot wall at x = 0.
  [hot_left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = left
    functor = ${T_hot_bc}
  []

  # Far-field solid at T_m = 0.
  [cold_right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = right
    functor = 0.0
  []
[]

[FunctorMaterials]
  [ins_fv]
    type = INSFVEnthalpyFunctorMaterial
    rho = ${rho_salt}
    cp = ${cp_salt}
    temperature = 'T'
  []
[]

[VectorPostprocessors]
  [horizontal_center]
    type = LineValueSampler
    start_point = '0 0 0'
    end_point = '0.5 0 0'
    num_points = ${fparse nx*0.5/Lx}
    variable = 'fl'
    sort_by = 'x'
    execute_on = 'FINAL'
  []
[]


[Executioner]
  type = PIMPLE

  dt = 1.25e-4

  # This corresponds to analytical final time tau_end = 0.25
  # because tau = t + t0.
  end_time = ${simulation_end_time}

  num_iterations = 60
  continue_on_max_its = true
  print_fields = false

  energy_system = energy_system

  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system'
  pressure_system = 'p_system'
  should_solve_momentum = false
  should_solve_pressure = false

  energy_l_abs_tol = 1e-18
  energy_l_tol = 1e-18

  energy_absolute_tolerance = 1e-18

  energy_equation_relaxation = 0.9

  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  [out_test]
    type = Exodus
    time_step_interval = 400
  []
[]
