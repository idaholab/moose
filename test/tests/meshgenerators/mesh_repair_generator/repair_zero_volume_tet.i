# A TET4 collapsed to a point: its four nodes lie within 1e-6 of the origin, sharing that corner
# with a healthy unit tetrahedron via 'fix_node_overlap'. The element is small in every dimension
# (tiny diameter), so the zero-volume repair merges its vertices onto a single node and deletes it,
# leaving the healthy tetrahedron intact and conformal (validated by MeshDiagnosticsGenerator).
[Mesh]
  [healthy]
    type = ElementGenerator
    nodal_positions = '0 0 0   1 0 0   0 1 0   0 0 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [point_tet]
    type = ElementGenerator
    input = healthy
    nodal_positions = '0 0 0   1e-6 0 0   0 1e-6 0   0 0 1e-6'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [repair]
    type = MeshRepairGenerator
    input = point_tet
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
