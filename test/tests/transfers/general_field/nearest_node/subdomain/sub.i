[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 0.2
    ymax = 0.2
  []
  [add_block]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x > 0.05 & y < 0.1'
    block_id = 1
  []
[]

[AuxVariables]
  [from_main]
    initial_condition = -1
  []
  [from_main_elem]
    order = CONSTANT
    family = MONOMIAL
    initial_condition = -1
  []
  [to_main]
    [InitialCondition]
      type = FunctionIC
      function = '3 + 2*x*x + 3*y*y*y'
    []
  []
  [to_main_elem]
    order = CONSTANT
    family = MONOMIAL
    [InitialCondition]
      type = FunctionIC
      function = '4 + 2*x*x + 3*y*y*y'
    []
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
  hide = 'to_main to_main_elem'
  execute_on = 'TIMESTEP_END'
[]
