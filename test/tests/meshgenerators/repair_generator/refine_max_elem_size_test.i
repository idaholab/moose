[Mesh]
  [elem_maker]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0
                       0 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = 'QUAD4'
  []
  [repair]
    type = MeshRepairGenerator
    input = elem_maker
    fix_elem_size = 3
  []
[]

[Outputs]
  exodus = true
[]

