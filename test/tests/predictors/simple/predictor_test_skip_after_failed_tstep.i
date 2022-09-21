# The purpose of this test is to test the simple predictor.
# The test is adjusted to produce a failed time step.
# The predictor option 'skip_after_failed_timestep' should suppress a prediction
# after the failed time step.

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

[Functions]
  [./ramp1]
    type = PiecewiseLinear
    x = '0      0.5     1'
    y = '0      1       4'
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./bot]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0.0
  [../]
  [./ss2_x]
    type = FunctionDirichletBC
    variable = u
    boundary = top
    function = ramp1
  [../]
[]

[Problem]
  type = FailingProblem
  fail_steps = '6'
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  nl_max_its = 15
  nl_rel_tol = 1e-14
  nl_abs_tol = 1e-14
  l_tol = 1e-14

  start_time = 0.0
  end_time = 1.0

  [./TimeStepper]
    type = ConstantDT
    dt = 0.1
    cutback_factor_at_failure = 0.5
  [../]

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
    skip_after_failed_timestep = true
  [../]
[]

[Postprocessors]
  [./final_residual]
    type = Residual
    residual_type = final
  [../]
  [./initial_residual_before]
    type = Residual
    residual_type = initial_before_preset
  [../]
  [./initial_residual_after]
    type = Residual
    residual_type = initial_after_preset
  [../]
[]

[Outputs]
  csv = true
[]
