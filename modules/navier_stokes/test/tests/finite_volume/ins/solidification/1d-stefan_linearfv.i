# Regression test for LinearFVPhaseChangeSource based on the 1D two-phase
# Stefan problem: melting driven by a hot wall at x = 0.
#
# Nondimensional properties rho = cp = k = L = 1 (Stefan number = 1). The
# sharp-interface analytical solution places the front at
#   s(tau) = 2 * lambda * sqrt(alpha * tau), lambda ~ 0.62, tau = t + t0,
# and the initial condition below is the analytical temperature profile at
# tau = t0, for which the front sits at s0 = 0.1.
#
# Compared to the full benchmark in examples/solidification/1d_stefan_T.i,
# this input is coarsened for speed and the mushy interval
# [T_solidus, T_liquidus] is widened so that it stays resolved by the coarse
# mesh (keeping the source term active and the gold file robust). This is a
# regression test, not a validation case.

rho = 1
cp = 1
k = 1
L = 1

T_solidus = 0
T_liquidus = 0.1

T_hot_bc = 1

# Time offset such that the initial front position is s0 = 0.1
t0 = 0.0065023282

Lx = 0.5
nx = 100

[Problem]
  kernel_coverage_check = false
  material_coverage_check = false
  previous_nl_solution_required = true
  linear_sys_names = 'energy_system p_system u_system'
[]

[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    nx = ${nx}
    xmin = 0.0
    xmax = ${Lx}
  []
[]

[Functions]
  # Analytical sharp-interface temperature profile at tau = t0
  [T_exact_initial]
    type = ParsedFunction
    expression = 'if(x < 0.1, 1.0 - erf(x / (2.0 * sqrt(${t0}))) / 0.6194595791, 0.0)'
  []
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    pressure = pressure
    rho = ${rho}
    p_diffusion_kernel = p_diffusion
  []
[]

[Variables]
  [T]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
  []

  # Dummy velocity/pressure variables: the PIMPLE executioner requires the
  # momentum and pressure systems to exist even though they are not solved.
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_system'
    initial_condition = 0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = 'p_system'
    initial_condition = 0
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
  [T_ic]
    type = FunctionIC
    variable = T
    function = T_exact_initial
  []
[]

[LinearFVKernels]
  # Pressure diffusion kernel kept only because the Rhie-Chow object
  # references it. The pressure solve is disabled in the Executioner.
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
  []

  # rho * cp * dT/dt
  [energy_time]
    type = LinearFVTimeDerivative
    variable = T
    factor = ${fparse rho * cp}
  []

  # div(k * grad(T))
  [energy_conduction]
    type = LinearFVDiffusion
    variable = T
    diffusion_coeff = ${k}
    use_nonorthogonal_correction = false
  []

  # rho * L * (df/dT) * dT/dt (apparent heat capacity)
  [energy_source]
    type = LinearFVPhaseChangeSource
    variable = T
    L = ${L}
    rho = ${rho}
    T_solidus = ${T_solidus}
    T_liquidus = ${T_liquidus}
    smoothing = smooth
  []
[]

[LinearFVBCs]
  # Hot wall at x = 0
  [hot_left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = left
    functor = ${T_hot_bc}
  []

  # Far-field solid at the melting temperature
  [cold_right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = right
    functor = 0.0
  []
[]

[FunctorMaterials]
  [enthalpy_material]
    type = INSFVEnthalpyFunctorMaterial
    rho = ${rho}
    cp = ${cp}
    temperature = 'T'
  []
[]

[Postprocessors]
  # Integral of the liquid fraction over the domain: in 1D this is the
  # position of the melting front, s(t).
  [front_position]
    type = ElementIntegralVariablePostprocessor
    variable = fl
  []
  [T_avg]
    type = ElementAverageValue
    variable = T
  []
[]

[Executioner]
  type = PIMPLE

  dt = 1e-3
  end_time = 1e-2

  num_iterations = 25
  continue_on_max_its = true
  print_fields = false

  energy_system = energy_system

  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system'
  pressure_system = 'p_system'
  should_solve_momentum = false
  should_solve_pressure = false

  energy_l_abs_tol = 1e-13
  energy_l_tol = 1e-13
  energy_absolute_tolerance = 1e-13

  energy_equation_relaxation = 0.9

  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  csv = true
[]
