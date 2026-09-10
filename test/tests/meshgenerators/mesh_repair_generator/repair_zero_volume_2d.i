# A QUAD4 collapsed to a point: its four nodes lie within 1e-6 of one corner, sharing that corner
# with a healthy unit quadrilateral. repair2DSlivers cannot absorb it (its longest edge has ~zero
# length), so the zero-volume repair removes it by merging its vertices onto a single node.
[Mesh]
  [healthy]
    type = ElementGenerator
    nodal_positions = '0 0 0   1 0 0   1 1 0   0 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [point_quad]
    type = ElementGenerator
    input = healthy
    nodal_positions = '0 0 0   1e-6 0 0   1e-6 1e-6 0   0 1e-6 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [repair]
    type = MeshRepairGenerator
    input = point_quad
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
