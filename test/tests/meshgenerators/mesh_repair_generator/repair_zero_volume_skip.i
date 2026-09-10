# Two point-collapsed QUAD4s sharing an edge (a degenerate cluster), attached to a healthy unit
# quadrilateral (which sets the mesh scale so the tiny quads are flagged). Merging either tiny quad's
# vertices would collapse the other to a repeated node, so the zero-volume repair conservatively
# leaves both in place and reports them (cluster removal is not implemented). A later routine may
# still act on them; this test only checks the zero-volume routine's "left in place" report.
[Mesh]
  [healthy]
    type = ElementGenerator
    nodal_positions = '0 0 0   1 0 0   1 1 0   0 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [point_quad_a]
    type = ElementGenerator
    input = healthy
    nodal_positions = '0 0 0   1e-6 0 0   1e-6 1e-6 0   0 1e-6 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [point_quad_b]
    type = ElementGenerator
    input = point_quad_a
    nodal_positions = '0 0 0   1e-6 0 0   1e-6 -1e-6 0   0 -1e-6 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [repair]
    type = MeshRepairGenerator
    input = point_quad_b
    fix_node_overlap = true
    fix_degenerate_elements = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
