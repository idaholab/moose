[Mesh]
  [cube]
    type = FileMeshGenerator
    file = 'mesh/cube.e'
  []
  second_order = true
  allow_renumbering = false
[]

[AuxVariables]
  [u_elem]
    [InitialCondition]
      type = FunctionIC
      function = 'x + y * x'
    []
    family = MONOMIAL
    order = CONSTANT
    block = 0
  []
  [u_nodal]
    [InitialCondition]
      type = FunctionIC
      function = 'x + y * x'
    []
  []
  [v_high]
    order = SECOND
  []
  [v_low]
  []
[]

[AuxKernels]
  [node_to_node_higher_order]
    type = ProjectionAux
    variable = v_high
    v = u_nodal
    execute_on = 'INITIAL TIMESTEP_END'
    # block restrict the lower D blocks 1 & 2 away
    block = 0
  []
  [elem_to_nodal]
    type = ProjectionAux
    variable = v_low
    v = u_elem
    execute_on = 'INITIAL TIMESTEP_END'
    # block restrict the lower D blocks 1 & 2 away
    block = 0
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
  exodus = true
  show = 'v_high v_low'
[]
