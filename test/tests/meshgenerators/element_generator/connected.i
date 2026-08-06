[Mesh]
  [./left]
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

  [./right]
    type = ElementGenerator
    input = left
    nodal_positions = '1 0 0
                       2 0 0
                       2 1 0
                       1 1 0'

    element_connectivity = '0 1 2 3'
    elem_type = "QUAD4"
    subdomain_id = 2
    subdomain_name = 'right'
  []

  [./connected]
    type = MeshRepairGenerator
    input = right
    fix_node_overlap = true
  []
[]

[Outputs]
  exodus = true
[]
