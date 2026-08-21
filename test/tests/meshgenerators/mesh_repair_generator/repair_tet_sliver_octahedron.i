# A valence-8 octahedral fan: an interior node V (shared by 8 tets) pushed close to one outer
# face, making a flat sliver. Several candidate edge collapses of V would invert a neighbor tet;
# the invertibility guard rejects those and commits a valid one, keeping the mesh conformal.
[Mesh]
  [t0]
    type = ElementGenerator
    nodal_positions = '0.32333 0.32333 0.32333  0 0 1  1 0 0  0 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t1]
    type = ElementGenerator
    input = t0
    nodal_positions = '0.32333 0.32333 0.32333  0 0 1  0 1 0  -1 0 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t2]
    type = ElementGenerator
    input = t1
    nodal_positions = '0.32333 0.32333 0.32333  0 0 1  -1 0 0  0 -1 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t3]
    type = ElementGenerator
    input = t2
    nodal_positions = '0.32333 0.32333 0.32333  0 0 1  0 -1 0  1 0 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t4]
    type = ElementGenerator
    input = t3
    nodal_positions = '0.32333 0.32333 0.32333  0 0 -1  0 1 0  1 0 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t5]
    type = ElementGenerator
    input = t4
    nodal_positions = '0.32333 0.32333 0.32333  0 0 -1  -1 0 0  0 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t6]
    type = ElementGenerator
    input = t5
    nodal_positions = '0.32333 0.32333 0.32333  0 0 -1  0 -1 0  -1 0 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t7]
    type = ElementGenerator
    input = t6
    nodal_positions = '0.32333 0.32333 0.32333  0 0 -1  1 0 0  0 -1 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [repair]
    type = MeshRepairGenerator
    input = t7
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
[Problem]
  solve = false
[]
[Executioner]
  type = Steady
[]
