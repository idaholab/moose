[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 2
  xmax = 5
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

[LinearFVKernels]
  [time]
    type = LinearFVTimeDerivative
    variable = 'u'
  []
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = 5
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = 2
  []
[]

[LinearFVBCs]
  [dir]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = "left right"
    functor = 12
  []
[]

[Executioner]
  type = Transient
  system_names = u_sys
  start_time = 0.0
  end_time = 19
  n_startup_steps = 2
  dtmax = 6.0
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 8
    dt = 1.0
  []
  verbose = true
  linear_convergence = much_logic
[]

[Convergence]
  [force_grow]
    type = IterationCountConvergence
    min_iterations = 0
    max_iterations = 4
    converge_at_max_iterations = true
  []
  [force_shrink]
    type = IterationCountConvergence
    min_iterations = 12
    max_iterations = 13
    converge_at_max_iterations = true
  []
  [much_logic]
    type = ParsedConvergence
    convergence_expression = 'if(time < 5, force_grow, force_shrink)'
    symbol_names = 'time force_grow force_shrink'
    symbol_values = 'time force_grow force_shrink'
  []
[]

[Postprocessors]
  [_dt]
    type = TimestepSize
  []
  [time]
    type = TimePostprocessor
  []
[]

[Outputs]
  csv = true
[]
