[Mesh]
  [copy1]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       2 2 0
                       2 0 0'
    element_connectivity = '0 1 2'
    elem_type = 'TRI3'
  []

  [copy2]
    type = ElementGenerator
    nodal_positions = '0 1 0
                       0 3 0
                       2 3 0
                       2 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = 'QUAD4'
  []

  [cmbn]
    type = CombinerGenerator
    inputs = 'copy1 copy2'
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = cmbn
    examine_element_types = INFO
  []
[]

[Outputs]
  exodus = true
[]
