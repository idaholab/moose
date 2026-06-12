# A thin QUAD4 sliver (a 2 x 0.01 strip) sharing its long edge (0,0)-(2,0) with a healthy
# triangle. Repairing the sliver removes it and promotes the triangle neighbor: inserting the two
# far vertices of the strip into the shared edge turns the triangle into a 5-sided polygon, while
# keeping the surface conformal. The result contains a polygon, which Exodus cannot store, so the
# repair is validated by MeshDiagnosticsGenerator (conformality + overlap) rather than by exodiff.
[Mesh]
  [quad_sliver]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       2 0 0
                       2 -0.01 0
                       0 -0.01 0'
    element_connectivity = '0 1 2 3'
    elem_type = QUAD4
  []
  [triangle_neighbor]
    type = ElementGenerator
    input = quad_sliver
    nodal_positions = '0 0 0
                       2 0 0
                       1 1 0'
    element_connectivity = '0 1 2'
    elem_type = TRI3
  []
  [repair]
    type = MeshRepairGenerator
    input = triangle_neighbor
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
