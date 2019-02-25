[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 100
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Functions]
  [./cosine_transition_fn]
    type = CosineTransitionFunction
    axis = y
    begin_coordinate = 0.1
    transition_width = 0.4
    begin_value = 0
    end_value = 100
  [../]
[]

[AuxVariables]
  [./cosine_transition]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./cosine_transition_kernel]
    type = FunctionAux
    variable = cosine_transition
    function = cosine_transition_fn
    execute_on = initial
  [../]
[]

[Outputs]
  exodus = true
  show = cosine_transition
  file_base = space_increase
  execute_on = initial
[]
