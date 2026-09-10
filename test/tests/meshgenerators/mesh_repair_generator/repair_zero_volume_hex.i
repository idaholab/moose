# A HEX8 collapsed to a point: its eight nodes lie within 1e-6 of one corner, sharing that corner
# with a healthy unit hexahedron via 'fix_node_overlap'. The zero-volume repair merges its eight
# vertices onto a single node and deletes it, leaving the healthy hexahedron intact.
[Mesh]
  [healthy]
    type = ElementGenerator
    nodal_positions = '0 0 0   1 0 0   1 1 0   0 1 0   0 0 1   1 0 1   1 1 1   0 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [point_hex]
    type = ElementGenerator
    input = healthy
    nodal_positions = '0 0 0   1e-6 0 0   1e-6 1e-6 0   0 1e-6 0   0 0 1e-6   1e-6 0 1e-6   1e-6 1e-6 1e-6   0 1e-6 1e-6'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [repair]
    type = MeshRepairGenerator
    input = point_hex
    fix_node_overlap = true
    fix_degenerate_elements = true
  []
  [diagnostics]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_element_volumes = ERROR
    examine_element_overlap = ERROR
    examine_non_conformality = ERROR
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
