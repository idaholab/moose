[Mesh]
  [copy1]
    type = ElementGenerator
    nodal_positions = '4 4 0
                       6 4 0
                       6 6 0
                       4 6 0
                       4 4 5
                       6 4 5
                       6 6 5
                       4 6 5'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = 'HEX8'
  []

  [gen]
    input = copy1
    type = RenameBlockGenerator
    old_block = "0"
    new_block = "1"
  []

  [copy2]
    type = ElementGenerator
    nodal_positions = '6 6.001 0
                       6 8 0
                       5 8 0
                       5 6.001 0
                       6 6.001 5
                       6 8 5
                       5 8 5
                       5 6.001 5'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = 'HEX8'
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
    nonconformal_tol = 0.01
  []
[]

[Outputs]
  exodus = true
[]
