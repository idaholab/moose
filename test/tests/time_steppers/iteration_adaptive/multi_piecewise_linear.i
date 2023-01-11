[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 50
  ny = 2
  xmax = 5
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Functions]
  [./temp_spike1]
    type = PiecewiseLinear
    x = '1 3 5'
    y = '1 4 4'
  [../]
  [./temp_spike2]
    type = PiecewiseLinear
    x = '0 2 4'
    y = '1 1 2'
  [../]

  [temp_spike]
    type = ParsedFunction
    expression = 'temp_spike1 + temp_spike2'
    symbol_names = 'temp_spike1 temp_spike2'
    symbol_values = 'temp_spike1 temp_spike2'
  []
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./dt]
    type = TimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = FunctionDirichletBC
    variable = u
    boundary = left
    function = temp_spike
  [../]
  [./right]
    type = NeumannBC
    variable = u
    boundary = right
    value = -1
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  start_time = 0
  end_time = 5
  verbose = true
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 10
    optimal_iterations = 10
    timestep_limiting_function = 'temp_spike1 temp_spike2'
  [../]
[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  csv = true
[]
