[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 1.0
  []
[]

[FVInterpolationMethods]
  [upwind]
    type = FVAdvectedUpwind
  []
  [average]
    type = FVGeometricAverage
  []
  [muscl_venkat]
    type = FVAdvectedVenkatakrishnanDeferredCorrection
    deferred_correction_factor = 1.0
  []
  [nvd_vanleer]
    type = FVAdvectedVanLeerWeightBased
    blending_factor = 1.0
  []
  [nvd_minmod]
    type = FVAdvectedMinmodWeightBased
    blending_factor = 1.0
  []
[]

[LinearFVKernels]
  [advection]
    type = LinearFVAdvection
    variable = u
    velocity = "0.5 0 0"
    advected_interp_method_name = upwind
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = source_func
  []
[]

[LinearFVBCs]
  [inflow]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left"
    functor = analytic_solution
  []
  [outflow]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = u
    boundary = "right"
    use_two_term_expansion = false
  []
[]

[Functions]
  [source_func]
    type = ParsedFunction
    expression = '0.5*x'
  []
  [analytic_solution]
    type = ParsedFunction
    expression = '0.5+0.5*x*x'
  []
[]

[Postprocessors]
  [error]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic_solution
    execute_on = FINAL
  []
  [h]
    type = AverageElementSize
    execute_on = FINAL
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
  l_tol = 1e-10
  multi_system_fixed_point=true
  multi_system_fixed_point_convergence=linear
  multi_system_fixed_point_relaxation_factor = 1.0
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -pc_factor_mat_solver_type -mat_mumps_icntl_14'
  petsc_options_value = 'lu       NONZERO               1e-12                     mumps                    50'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = FINAL
  []
[]
