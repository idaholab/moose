# This test tests the ChangeOverTimestepPostprocessor, which computes the change
# in a postprocessor value over a timestep. This test creates a time-dependent
# function postprocessor and then computes its change over a timestep. The
# FE problem used here is a dummy problem and has no effect on the test.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 5
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./time_derivative]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
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

[Executioner]
  type = Transient

  start_time = 0.0
  dt = 1.0
  num_steps = 2
[]

[Functions]
  [./my_function]
    type = ParsedFunction
    value = '1 + t * t'
  [../]
[]

[Postprocessors]
  [./my_postprocessor]
    type = FunctionValuePostprocessor
    function = my_function
    execute_on = 'initial timestep_end'
  [../]
  [./change_over_timestep]
    type = ChangeOverTimestepPostprocessor
    postprocessor = my_postprocessor
  [../]
[]

[Outputs]
  csv = true
[]
