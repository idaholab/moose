# Verification of the pressure work functor, Dp/Dt = dp/dt + u . grad(p), against its analytic
# value, with the pressure carried by a solved linear finite volume variable so that the gradient
# is the reconstructed one rather than an analytic function's.
#
# The pressure solves Laplace's equation with Dirichlet data taken from a linear field. That field
# is harmonic, so it is the exact solution, and a cell centred finite volume discretisation
# reproduces a linear field and its reconstructed gradient exactly on any mesh.
#
#   p     = 3x + 2y            -> grad(p) = (3, 2),  <p> = 2.5 over the unit square
#   u     = (0.5, 0.25)        prescribed, the flow is not solved
#
#   u . grad(p) = 0.5 * 3 + 0.25 * 2 = 2.0
#
# The variable starts at zero, so the two steps below exercise the two halves of the term
# separately, with dt = 1 and a first order integrator:
#
#   step 1:  dp/dt = (p - 0)/dt = p,  <dp/dt> = 2.5   ->  Dp/Dt = 2.5 + 2.0 = 4.5
#   step 2:  the field no longer moves, dp/dt = 0     ->  Dp/Dt = 0.0 + 2.0 = 2.0
#
# The second material repeats the evaluation with the transient part switched off and must report
# 2.0 at both steps. That is what the mixture form relies on when the term is assembled in more
# than one piece: only one piece may carry dp/dt.

u_x = 0.5
u_y = 0.25

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 4
    ny = 4
  []
[]

[Problem]
  linear_sys_names = 'p_system'
[]

[Variables]
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = p_system
  []
[]

[Functions]
  [p_exact]
    type = ParsedFunction
    expression = '3*x + 2*y'
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = pressure
    diffusion_coeff = 1.0
    use_nonorthogonal_correction = false
  []
[]

[LinearFVBCs]
  [all]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = pressure
    boundary = 'left right top bottom'
    functor = p_exact
  []
[]

[FunctorMaterials]
  [pressure_work]
    type = NSFVPressureWorkFunctorMaterial
    pressure = pressure
    u = ${u_x}
    v = ${u_y}
    pressure_work_name = pressure_work
  []
  [pressure_work_no_transient]
    type = NSFVPressureWorkFunctorMaterial
    pressure = pressure
    u = ${u_x}
    v = ${u_y}
    include_time_derivative = false
    pressure_work_name = pressure_work_advective
  []
[]

[AuxVariables]
  # The functor is evaluated at cell centroids, which is where the source kernel consuming it
  # evaluates it too. A functor postprocessor would probe it at quadrature points instead.
  [pressure_work_aux]
    type = MooseVariableFVReal
  []
  [pressure_work_advective_aux]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [compute_pressure_work]
    type = FunctorAux
    variable = pressure_work_aux
    functor = pressure_work
    execute_on = 'TIMESTEP_END'
  []
  [compute_pressure_work_advective]
    type = FunctorAux
    variable = pressure_work_advective_aux
    functor = pressure_work_advective
    execute_on = 'TIMESTEP_END'
  []
[]

[Postprocessors]
  # The mean of 3x + 2y over the unit square, a check that the solved field is the intended one
  [p_avg]
    type = ElementAverageValue
    variable = pressure
    execute_on = 'TIMESTEP_END'
  []
  [pressure_work_avg]
    type = ElementAverageValue
    variable = pressure_work_aux
    execute_on = 'TIMESTEP_END'
  []
  [pressure_work_advective_avg]
    type = ElementAverageValue
    variable = pressure_work_advective_aux
    execute_on = 'TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 2
  system_names = p_system
  l_tol = 1e-10
[]

[Outputs]
  csv = true
[]
