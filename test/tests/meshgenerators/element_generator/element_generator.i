[Mesh]
  [./eg]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0'

    element_connectivity = '0 1 2'
    elem_type = "TRI3"
  []
[]

[Outputs]
  exodus = true
[]
