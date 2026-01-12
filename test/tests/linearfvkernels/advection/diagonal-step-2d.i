[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 50
  []
[]

[Problem]
  linear_sys_names = 'u_sys'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    solver_sys = 'u_sys'
    initial_condition = 0.0
  []
[]

[InterpolationMethods]
  [upwind]
    type = FVAdvectedUpwind
  []
  [average]
    type = FVGeometricAverage
  []
  [vanleer]
    type = FVAdvectedVanLeerDeferredCorrection
  []
  [vanleer2]
    type = FVAdvectedVanLeerWeightBased
    blending_factor = 0.6
  []
[]

[LinearFVKernels]
  [advection]
    type = LinearFVAdvection
    variable = u
    velocity = "1 1 0"
    advected_interp_method_name = vanleer2
  []
[]

[LinearFVBCs]
  [left_inflow]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = left
    functor = 0
  []
  [bottom_inflow]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = bottom
    functor = 1
  []
  [outflow]
    type = LinearFVAdvectionDiffusionOutflowBC
    variable = u
    boundary = 'right top'
    use_two_term_expansion = true
  []
[]

[VectorPostprocessors]
  [diag_sample]
    type = LineValueSampler
    variable = u
    start_point = '1.0 0 0'
    end_point = '0 1.0 0'
    num_points = 200
    sort_by = id
    warn_discontinuous_face_values = false
    execute_on = TIMESTEP_END
  []
[]

[Convergence]
  [linear]
    type = IterationCountConvergence
    max_iterations = 200
    converge_at_max_iterations = true
  []
[]

[Executioner]
  type = Steady
  system_names = u_sys
  l_tol = 1e-12
  multi_system_fixed_point = true
  multi_system_fixed_point_convergence = linear
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu       NONZERO               1e-10'
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = TIMESTEP_END
[]
