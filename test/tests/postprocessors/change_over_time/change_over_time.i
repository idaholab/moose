# This test tests the ChangeOverTimePostprocessor, which computes the change
# in a postprocessor value with respect to the previous value or with respect to
# the initial value. This test creates a time-dependent function postprocessor
# and then computes its change over a timestep. The FE problem used here is a
# dummy problem and has no effect on the test.

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
    expression = '1 + t * t'
  [../]
[]

[Postprocessors]
  [./my_postprocessor]
    type = FunctionValuePostprocessor
    function = my_function
    execute_on = 'initial timestep_end'
  [../]
  [./change_over_time]
    type = ChangeOverTimePostprocessor
    postprocessor = my_postprocessor
    change_with_respect_to_initial = false
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  file_base = 'change_over_time_previous'
  csv = true
[]
