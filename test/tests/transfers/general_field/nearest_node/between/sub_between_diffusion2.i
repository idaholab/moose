[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    # partial overlap but also, no equidistant points
    xmin = 0.1111
    ymin = 0.3333
    xmax = 1.211111
    ymax = 1.222222
  []
  [block1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0.4 0.6 0'
    # extends beyond to grab the boundary
    top_right = '2 2 0'
  []
[]

[AuxVariables]
  [sent_nodal]
    [InitialCondition]
      type = FunctionIC
      function = '3 + 2*x*x + 3*y*y*y'
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
      function = '4 + 2*x*x + 3*y*y*y'
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
