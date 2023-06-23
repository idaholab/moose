[Mesh]
  [copy1]
    type = ElementGenerator
    nodal_positions = '2 2 0
                       4 2 0
                       4 4 0
                       2 4 0'
    element_connectivity = '0 1 2 3'
    elem_type = 'QUAD4'
  []
  [gen]
    input = copy1
    type = RenameBlockGenerator
    old_block = "0"
    new_block = "1"
  []

  [copy2]
    type = ElementGenerator
    nodal_positions = '1 3 0
                       5 2 0
                       5 4 0'

    element_connectivity = '0 1 2'
    elem_type = 'TRI3'
  []

  [cmbn]
    type = CombinerGenerator
    inputs = 'gen copy2'
  []

  [diag]
    type = MeshDiagnosticsGenerator
    input = cmbn
    examine_element_overlap = true
  []
[]

[Outputs]
  exodus = true
[]
