[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 10
[]

[Problem]
  solve = false
[]

[Functions]
  [transition_fn]
    type = CosineTransitionFunction
    axis = t
    transition_center = 5
    transition_width = 4
    function1 = 0
    function2 = 100
  []
[]

[Postprocessors]
  [transition]
    type = FunctionValuePostprocessor
    function = transition_fn
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
  file_base = time_weighted
[]
