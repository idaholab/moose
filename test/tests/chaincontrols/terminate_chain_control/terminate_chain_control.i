# Tests TerminateChainControl

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Functions]
  [test_fn]
    type = PiecewiseLinear
    x = '0 1 2 3'
    y = '0 0 1 0'
  []
[]

[ChainControls]
  [test_fn_ctrl]
    type = GetFunctionValueChainControl
    function = test_fn
  []
  [input_ctrl]
    type = RealToBoolChainControl
    input = test_fn_ctrl:value
  []
  [terminate_ctrl]
    type = TerminateChainControl
    input = input_ctrl:value
    terminate_on_true = true
    throw_error = true
    termination_message = 'Hasta la vista, baby'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 3
[]
