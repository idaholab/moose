[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
  xmax = 3
[]

[Functions]
  [./fn]
    type = PiecewiseLinear
    axis = x
    x = '0 2'
    y = '1.01 2.99'
  [../]
[]

[AuxVariables]
  [./a]
  [../]
[]

[AuxKernels]
  [./a_ak]
    type = FunctionAux
    variable = a
    function = fn
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Postprocessors]
  [./value1]
    type = PointValue
    variable = a
    point = '0 0 0'
  [../]

  [./value2]
    type = PointValue
    variable = a
    point = '1 0 0'
  [../]

  [./value3]
    type = PointValue
    variable = a
    point = '2 0 0'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.1
  solve_type = NEWTON
[]

[Outputs]
  [./csv]
    type = CSV
    file_base = csv_validation_tester_01
    execute_on = 'final'
  [../]
[]
