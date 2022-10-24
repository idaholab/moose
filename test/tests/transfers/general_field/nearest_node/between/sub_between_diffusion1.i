[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
  []
  [block1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.5 0.5 0'
  []
[]

[AuxVariables]
  [sent_nodal]
    [InitialCondition]
      type = FunctionIC
      function = '1 + 2*x*x + 3*y*y*y'
    []
  []
  [received_nodal]
    initial_condition = -1
  []
  [sent_elem]
    family = MONOMIAL
    order = CONSTANT
    [InitialCondition]
      type = FunctionIC
      function = '2 + 2*x*x + 3*y*y*y'
    []
  []
  [received_elem]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = -1
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
  hide = 'sent_nodal sent_elem'
  execute_on = 'TIMESTEP_END'
[]
