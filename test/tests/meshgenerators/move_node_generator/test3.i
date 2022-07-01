[Mesh]
  [eg]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0'

    element_connectivity = '0 1 2'
    elem_type = "TRI3"
  []
  [modifyNode]
    type = MoveNodeGenerator
    input = eg
    node_id = '0 1 2'
    shift_position = '-1 0 0
                       0 1 0
                       0 2 0'
  []
[]

[Outputs]
  exodus = true
[]
