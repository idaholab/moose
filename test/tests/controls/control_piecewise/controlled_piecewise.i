[Mesh]
  [./generated]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0
    xmax = 1
    nx = 10
  [../]
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./u]
    initial_condition = 0.1
  [../]
[]

[AuxVariables]
  [./v]
  [../]
  [./x]
  [../]
[]

[ICs]
  [./x_ic]
    type = FunctionIC
    variable = x
    function = 'x'
  [../]
[]

[AuxKernels]
  [./v_aux]
    type = FunctionAux
    variable = v
    function = func
  [../]
[]

[Controls]
  [./func_control]
    type = RealFunctionControl
    parameter = '*/*/scale_factor'
    function = '2'
    execute_on = 'initial'
  [../]
[]

[Materials]
  [./mat]
    type = PiecewiseLinearInterpolationMaterial
    property = matprop
    variable = x
    x = '0 1'
    y = '0 10'
    outputs = all
  [../]
[]

[Functions]
  [./func]
    type = PiecewiseLinear
    x = '0 1'
    y = '0 10'
    axis = x
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
