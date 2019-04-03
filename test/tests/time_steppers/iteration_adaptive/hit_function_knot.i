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
  [./knot]
    type = PiecewiseLinear
    x = '0 1 2'
    y = '0 0 0'
  [../]
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
    type = DirichletBC
    variable = u
    boundary = left
    value = 10
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
  start_time = 0.0
  end_time = 2.0
  timestep_tolerance = 0.3
  verbose = true
  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.9
    optimal_iterations = 10
  [../]
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]
[]

[Outputs]
  exodus = true
[]
