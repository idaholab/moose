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
  [./timestep_fn]
    type = PiecewiseLinear
    x = '0.  40.'
    y = '10. 1. '
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
  end_time = 40.0
  n_startup_steps = 2
  dtmax = 6.0
  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 10
    timestep_limiting_postprocessor = timestep_pp
    dt = 1.0
  [../]
[]

[Postprocessors]
  [./_dt]
    type = TimestepSize
  [../]

# Just use a simple postprocessor to test capability to limit the time step length to the postprocessor value
  [./timestep_pp]
    type = FunctionValuePostprocessor
    function = timestep_fn
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  checkpoint = true
[]
