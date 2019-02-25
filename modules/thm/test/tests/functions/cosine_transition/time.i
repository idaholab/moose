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
  [./cosine_transition_fn]
    type = CosineTransitionFunction
    axis = t
    begin_coordinate = 3
    transition_width = 4
    begin_value = 0
    end_value = 100
  [../]
[]

[Postprocessors]
  [./cosine_transition]
    type = FunctionValuePostprocessor
    function = cosine_transition_fn
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  csv = true
[]
