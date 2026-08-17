# The steps from t = 1.0 to t = 1.5 and from t = 1.5 to t = 2.0 are skipped
# because the latter would use the solution at t = 1.0 in the quadratic history.

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
  dt = 0.5
  end_time = 2.0

  [Predictor]
    type = QuadraticPredictor
    scale = 1.0
    skip_times_old = '1.0'
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
