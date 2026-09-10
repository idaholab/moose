# A QUAD4 with a short edge (vertex 3 within 1e-7 of vertex 0) next to a healthy quadrilateral with
# which it shares a full-length edge (merged by fix_node_overlap). The short edge is collapsed,
# reducing the degenerate quad to a triangle (QUAD4 -> TRI3); the healthy quad is untouched and the
# mesh stays conformal (validated by MeshDiagnosticsGenerator).
[Mesh]
  [quad_short]
    type = ElementGenerator
    nodal_positions = '0 0 0   1 0 0   1 1 0   0 1e-7 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [quad_healthy]
    type = ElementGenerator
    input = quad_short
    nodal_positions = '1 0 0   2 0 0   2 1 0   1 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [repair]
    type = MeshRepairGenerator
    input = quad_healthy
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
