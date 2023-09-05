[Mesh]
  [crooked]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 1 0
                       0 1 0
                       1 0 0'
    element_connectivity = '0 1 2 3'
    elem_type = 'QUAD4'
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = crooked
    check_local_jacobian = INFO
  []
[]

[Outputs]
  exodus = true
[]
