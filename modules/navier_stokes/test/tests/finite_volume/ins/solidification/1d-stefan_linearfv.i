# Regression test for LinearFVPhaseChangeSource based on the 1D two-phase
# Stefan problem: melting driven by a hot wall at x = 0.
#
# Nondimensional properties rho = cp = k = L = 1 (Stefan number = 1). The
# sharp-interface analytical solution places the front at
#
#   s(tau) = 2 * lambda * sqrt(alpha * tau),
#
# where lambda ~ 0.62 and tau = t + t0. The initial condition is the
# analytical temperature profile at tau = t0, for which the front is at
# s0 = 0.1.
#
# Compared with examples/solidification/1d_stefan_T.i, this input is
# coarsened for speed and uses a wider mushy interval so that the phase-change
# source remains resolved on the coarse mesh. This is a regression test, not
# a validation case.

rho = 1
cp = 1
k = 1
L = 1

T_solidus = 0
T_liquidus = 0.1

T_hot_bc = 1

# Time offset corresponding to an initial front position s0 = 0.1.
t0 = 0.0065023282

Lx = 0.5
nx = 100

[Problem]
  linear_sys_names = 'energy_system'

  # LinearFVPhaseChangeSource evaluates its temperature-dependent
  # coefficient using the previous nonlinear-iteration solution.
  previous_nl_solution_required = true
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
  # Analytical sharp-interface temperature profile at tau = t0.
  [T_exact_initial]
    type = ParsedFunction
    expression = 'if(x < 0.1, 1.0 - erf(x / (2.0 * sqrt(${t0}))) / 0.6194595791, 0.0)'
  []
[]

[Variables]
  [T]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
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
    T_liquidus = ${T_liquidus}
    T_solidus = ${T_solidus}
    execute_on = TIMESTEP_END
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

  [energy_time]
    type = LinearFVTimeDerivative
    variable = T
    factor = ${fparse rho * cp}
  []
  [energy_conduction]
    type = LinearFVDiffusion
    variable = T
    diffusion_coeff = ${k}
    use_nonorthogonal_correction = false
  []
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
  # Hot wall at x = 0.
  [hot_left]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = left
    functor = ${T_hot_bc}
  []

  # Far-field solid at the melting temperature.
  [cold_right]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = T
    boundary = right
    functor = 0.0
  []
[]

[Postprocessors]
  # Integral of the liquid fraction. In this 1D problem, it approximates
  # the position of the melting front.
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
  type = Transient

  dt = 1e-3
  end_time = 1e-2

  system_names = 'energy_system'

  l_abs_tol = 1e-13
  l_tol = 1e-13
  l_max_its = 1000

  petsc_options_iname = '-energy_system_pc_type -energy_system_pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

  # Repeat the temperature solve so that the lagged apparent heat-capacity
  # coefficient is updated within each time step.
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = phase_change_fixed_point
[]

[Convergence]
  # Keep the number of coefficient-update iterations deterministic for the
  # regression gold files.
  [phase_change_fixed_point]
    type = IterationCountConvergence
    max_iterations = 25
    converge_at_max_iterations = true
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
