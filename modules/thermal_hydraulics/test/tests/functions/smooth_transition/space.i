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
  [transition_fn]
    type = CosineTransitionFunction
    axis = y
    transition_center = 0.3
    transition_width = 0.4
    function1 = 0
    function2 = 100
  []
[]

[AuxVariables]
  [transition]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [transition_kernel]
    type = FunctionAux
    variable = transition
    function = transition_fn
    execute_on = initial
  []
[]

[Outputs]
  exodus = true
  show = transition
  file_base = space_weighted
  execute_on = initial
[]
