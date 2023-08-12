[Mesh]
  [cube]
    type = ElementGenerator
    nodal_positions = '4 4 0
                       6 4 0
                       6 6 0
                       4 6 0
                       4 4 5
                       6 4 3
                       6 6 5
                       4 6 5'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = 'HEX8'
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = cube
    examine_nonplanar_sides = INFO
  []
[]

[Outputs]
  exodus = true
[]
