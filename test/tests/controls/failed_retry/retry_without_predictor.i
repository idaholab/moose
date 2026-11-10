# Time step 3 fails. Controls executed on FAILED turn the predictor off and keep the time step size,
# so the step is repeated once with the same dt and without the prediction.
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = FunctionDirichletBC
    variable = u
    boundary = left
    function = t
  []
[]

[Problem]
  type = FailingProblem
  fail_steps = '3'
[]

[Executioner]
  type = Transient
  num_steps = 4
  dt = 0.1
  dtmin = 0.01
  [Predictor]
    type = SimplePredictor
    scale = 1
  []
[]

[Postprocessors]
  [failures_this_step]
    type = NumFailedTimeSteps
    current_time_step_only = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [dt]
    type = TimestepSize
  []
[]

[Functions]
  [cutback]
    type = ParsedFunction
    expression = 'if(n < 1, 1, 0.5)'
    symbol_names = n
    symbol_values = failures_this_step
  []
[]

[Controls]
  [predictor_off]
    type = BoolFunctionControl
    parameter = '*/*/apply'
    function = 0
    execute_on = FAILED
  []
  [keep_dt]
    type = RealFunctionControl
    parameter = '*/*/cutback_factor_at_failure'
    function = cutback
    execute_on = FAILED
  []
  [predictor_on]
    type = BoolFunctionControl
    parameter = '*/*/apply'
    function = 1
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  csv = true
[]
