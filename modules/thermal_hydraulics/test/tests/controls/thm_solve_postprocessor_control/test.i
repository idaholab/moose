[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Functions]
  [active_fn]
    type = PiecewiseConstant
    direction = right
    xy_data = '
      0.2 0
      0.4 1
      0.6 0'
  []
[]

[Postprocessors]
  [active]
    type = FunctionValuePostprocessor
    function = active_fn
  []
[]

[Components]
[]

[ControlLogic]
  [solve_on_off]
    type = THMSolvePostprocessorControl
    postprocessor = active
  []
[]


[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 6
  abort_on_solve_fail = true
[]

[Outputs]
  csv = true
[]
