# A "tent": a base triangle (A,B,C) and apex U, with an interior node V placed just above the base
# centroid. The tet (A,B,C,V) is a flat INTERIOR sliver (V is surrounded by the four tets, none of
# its faces is external). Repairing it collapses the interior node V onto a base vertex, deleting
# the sliver and reshaping the rest into a single healthy tetrahedron (A,B,C,U). The four tets are
# built separately (sharing nodes) and merged by 'fix_node_overlap'. The result is validated by
# MeshDiagnosticsGenerator (conformality, overlap, and volume) and, being all-tet, is exodiff'able.
[Mesh]
  [tet0]
    type = ElementGenerator
    nodal_positions = '0 0 0   1 0 0   0.5 0.866 0   0.5 0.289 0.01'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [tet1]
    type = ElementGenerator
    input = tet0
    nodal_positions = '0 0 0   1 0 0   0.5 0.289 0.01   0.5 0.289 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [tet2]
    type = ElementGenerator
    input = tet1
    nodal_positions = '1 0 0   0.5 0.866 0   0.5 0.289 0.01   0.5 0.289 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [tet3]
    type = ElementGenerator
    input = tet2
    nodal_positions = '0.5 0.866 0   0 0 0   0.5 0.289 0.01   0.5 0.289 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [repair]
    type = MeshRepairGenerator
    input = tet3
    fix_node_overlap = true
    fix_sliver_elements = true
  []
  [diagnostics]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_non_conformality = ERROR
    examine_element_overlap = ERROR
    examine_element_volumes = ERROR
  []
[]
