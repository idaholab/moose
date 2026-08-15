[Mesh]
  [eg]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0
                       0 1 0'

    element_connectivity = '0 1 2 3'
    elem_type = "QUAD4"
    subdomain_id = 1
    subdomain_name = 'left'
  []

  [eg2]
    type = ElementGenerator
    input = eg
    nodal_positions = '2 0 0
                       3 0 0
                       3 1 0
                       2 1 0'

    element_connectivity = '0 1 2 3'
    elem_type = "QUAD4"
    subdomain_id = 2
    subdomain_name = 'right'
  []
[]

[Outputs]
  exodus = true
[]
