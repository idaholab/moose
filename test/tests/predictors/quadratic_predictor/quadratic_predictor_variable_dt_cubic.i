# This input checks nonuniform time-step quadratic prediction on a solution
# history that is not exactly quadratic in time.

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
    function = 't*t*t'
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
    type = TimeSequenceStepper
    time_sequence = '0 0.25 0.75 1.5 2.0'
  []

  [Predictor]
    type = QuadraticPredictor
    scale = 1.0
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
