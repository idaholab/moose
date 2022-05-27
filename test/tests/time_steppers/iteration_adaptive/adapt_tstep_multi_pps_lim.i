[Mesh]
  [generated_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 2
    xmax = 5
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Functions]
  [timestep_fn1]
    type = PiecewiseLinear
    x = '0  40'
    y = '10 1'
  []
  [timestep_fn2]
    type = PiecewiseLinear
    x = '0  40'
    y = '2  5'
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [dt]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 10
  []
  [right]
    type = NeumannBC
    variable = u
    boundary = right
    value = -1
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  start_time = 0.0
  end_time = 40.0
  dtmax = 6.0
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 10
    timestep_limiting_postprocessor = 'timestep_pp1 timestep_pp2'
    dt = 1.0
  []
[]

[Postprocessors]
  [_dt]
    type = TimestepSize
  []
  [timestep_pp1]
    type = FunctionValuePostprocessor
    function = timestep_fn1
  []
  [timestep_pp2]
    type = FunctionValuePostprocessor
    function = timestep_fn2
  []
[]

[Outputs]
  csv = true
[]
