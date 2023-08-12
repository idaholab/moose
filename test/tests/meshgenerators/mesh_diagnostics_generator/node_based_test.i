[Mesh]
  [copy1]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       0 2 0
                       2 2 0
                       2 0 0'
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
    nodal_positions = '0 3 0
                       0 2 0
                       1 1 0
                       2 3 0'

    element_connectivity = '0 1 2 3'
    elem_type = 'QUAD4'
  []

  [cmbn]
    type = CombinerGenerator
    inputs = 'gen copy2'
  []

  [diag]
    type = MeshDiagnosticsGenerator
    input = cmbn
    examine_element_overlap = INFO
  []
[]

[Outputs]
  exodus = true
[]
