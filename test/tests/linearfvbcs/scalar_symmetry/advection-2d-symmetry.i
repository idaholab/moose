vel_x = -0.1
two_term_bc=true

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    xmin = 0
    xmax = ${fparse pi/3}
    ymin = 0
    ymax = ${fparse pi/3}
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 0.02
  []
[]

[FVInterpolationMethods]
  [average]
    type = FVGeometricAverage
  []
[]

[Functions]
  [u_exact]
    type = ParsedFunction
    expression = 'cos(x)*cos(y)'
  []
  [source_fn]
    type = ParsedFunction
    expression = '-${vel_x}*sin(x)*cos(y)'
  []
[]

[LinearFVKernels]
  [advection]
    type = LinearFVAdvection
    variable = u
    velocity = "${vel_x} 0 0"
    advected_interp_method_name = average
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_fn
  []
[]

[LinearFVBCs]
  [symm]
    type = LinearFVAdvectionDiffusionScalarSymmetryBC
    variable = u
    boundary = "bottom"
    use_two_term_expansion = ${two_term_bc}
  []
  [outflow]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = u
    boundary = "left"
    use_two_term_expansion = ${two_term_bc}
  []
  [dir]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "right top"
    functor = u_exact
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = 'TIMESTEP_END'
  []
  [error]
    type = ElementL2FunctorError
    approximate = u
    exact = u_exact
    execute_on = 'TIMESTEP_END'
  []
[]

[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 1
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-9
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       NONZERO               1e-10'
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = linear
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
  exodus = true
[]
