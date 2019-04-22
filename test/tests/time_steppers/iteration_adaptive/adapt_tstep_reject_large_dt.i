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
    type = PiecewiseConstant
    x = '0.   10.0'
    y = '10.0 1.0'
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
  end_time = 12.0
  dtmax = 10.0
  dtmin = 0.1
  [./TimeStepper]
    type = IterationAdaptiveDT
    timestep_limiting_postprocessor = timestep_pp
    reject_large_step = true
    reject_large_step_threshold = 0.5
    dt = 3.0
    growth_factor = 1.0
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
