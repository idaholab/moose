[Mesh]
  [./eg]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0
                       0 1 0'

    element_connectivity = '0 1 2 3'
    elem_type = "QUAD4"
  []

  [./eg2]
    type = ElementGenerator
    input = eg
    nodal_positions = '0 0 0
                       -1 0 0
                       -1 -1 0
                       0 -1 0'

    element_connectivity = '0 1 2 3'
    elem_type = "QUAD4"
  []
[]

[Outputs]
  exodus = true
[]
