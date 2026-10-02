# This input fails the first step where quadratic prediction would be available.
# The repeated step should skip prediction and must not shift predictor history twice.

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 3
  ny = 3
[]

[Problem]
  type = FailingProblem
  fail_steps = '3'
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0.0
  []
  [top]
    type = FunctionDirichletBC
    variable = u
    boundary = top
    function = 't*t'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  nl_max_its = 15
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-14

  start_time = 0.0
  end_time = 2.0

  [TimeStepper]
    type = ConstantDT
    dt = 0.5
    cutback_factor_at_failure = 0.5
  []

  [Predictor]
    type = QuadraticPredictor
    scale = 1.0
    skip_after_failed_timestep = true
  []
[]

[Postprocessors]
  [final_residual]
    type = Residual
    residual_type = FINAL
  []
  [initial_residual]
    type = Residual
    residual_type = INITIAL
  []
  [num_nonlinear_iterations]
    type = NumNonlinearIterations
  []
[]

[Outputs]
  csv = true
[]
