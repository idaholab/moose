# Exercises the generalized sliver repair across element-type combinations. Each pair below is a
# thin sliver sharing its long edge with a healthy neighbor; the duplicated shared-edge nodes are
# merged by 'fix_node_overlap'. Repairing absorbs each sliver into its neighbor, promoting the
# neighbor to a quad or polygon. The results contain polygons (which Exodus cannot store), so the
# repair is validated by MeshDiagnosticsGenerator (conformality + overlap) rather than by exodiff.
[Mesh]
  # QUAD sliver + QUAD neighbor -> hexagon
  [quad_sliver]
    type = ElementGenerator
    nodal_positions = '0 0 0  2 0 0  2 -0.01 0  0 -0.01 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [quad_neighbor]
    type = ElementGenerator
    input = quad_sliver
    nodal_positions = '0 0 0  2 0 0  2 1 0  0 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  # TRI sliver + QUAD neighbor -> pentagon (promotes, since the neighbor is not a triangle)
  [tri_sliver]
    type = ElementGenerator
    input = quad_neighbor
    nodal_positions = '0 5 0  2 5 0  1 4.99 0'
    element_connectivity = '0 1 2'
    elem_type = TRI3
  []
  [tri_quad_neighbor]
    type = ElementGenerator
    input = tri_sliver
    nodal_positions = '0 5 0  2 5 0  2 6 0  0 6 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  # POLYGON (pentagon) sliver + QUAD neighbor -> heptagon
  [poly_sliver]
    type = ElementGenerator
    input = tri_quad_neighbor
    nodal_positions = '0 10 0  2 10 0  2 9.99 0  1 9.99 0  0 9.99 0'
    element_connectivity = '0 1 2 3 4'
    elem_type = C0POLYGON
  []
  [poly_quad_neighbor]
    type = ElementGenerator
    input = poly_sliver
    nodal_positions = '0 10 0  2 10 0  2 11 0  0 11 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  # POLYGON (pentagon) sliver + POLYGON (pentagon) neighbor -> octagon
  [poly_sliver2]
    type = ElementGenerator
    input = poly_quad_neighbor
    nodal_positions = '0 15 0  2 15 0  2 14.99 0  1 14.99 0  0 14.99 0'
    element_connectivity = '0 1 2 3 4'
    elem_type = C0POLYGON
  []
  [poly_neighbor]
    type = ElementGenerator
    input = poly_sliver2
    nodal_positions = '0 15 0  2 15 0  2 16 0  1 16.5 0  0 16 0'
    element_connectivity = '0 1 2 3 4'
    elem_type = C0POLYGON
  []
  [repair]
    type = MeshRepairGenerator
    input = poly_neighbor
    fix_node_overlap = true
    fix_sliver_triangles = true
  []
  [diagnostics]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_non_conformality = ERROR
    examine_element_overlap = ERROR
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
