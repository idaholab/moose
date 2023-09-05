[Mesh]
  [copy1]
    type = ElementGenerator
    nodal_positions = '4 2 0
                       8 2 0
                       8 5 0
                       4 5 0'
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
    nodal_positions = '8.001 2 0
                       12 2 0
                       12 5 0
                       8.001 3.5 0'

    element_connectivity = '0 1 2 3'
    elem_type = 'QUAD4'
  []

  [stitched]
    type = StitchedMeshGenerator
    inputs = 'gen copy2'
    stitch_boundaries_pairs = '0 1'
  []

  [diag]
    type = MeshDiagnosticsGenerator
    input = stitched
    examine_non_conformality = INFO
    nonconformal_tol = 0.1
  []
[]

[Outputs]
  exodus = true
[]
