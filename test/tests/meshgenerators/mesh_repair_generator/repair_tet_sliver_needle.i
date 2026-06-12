# A high-valence (valence-8) interior needle/spindle sliver: a ring of tets share a very short
# central edge G-K (two nodes nearly adjacent). Collapsing that short edge removes all the
# needle slivers at once and reshapes the surrounding tets, keeping a valid all-tet mesh.
# Validated by MeshDiagnosticsGenerator (conformality, overlap, volume).
[Mesh]
  [t0]
    type = ElementGenerator
    nodal_positions = '0 0 0   0 0 0.005   1.0 0.0 0.0025   6.123233995736766e-17 1.0 0.0025'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t1]
    type = ElementGenerator
    input = t0
    nodal_positions = '0 0 0.005   1.0 0.0 0.0025   6.123233995736766e-17 1.0 0.0025   0 0 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t2]
    type = ElementGenerator
    input = t1
    nodal_positions = '0 0 0   1.0 0.0 0.0025   0 0 -1   6.123233995736766e-17 1.0 0.0025'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t3]
    type = ElementGenerator
    input = t2
    nodal_positions = '0 0 0   0 0 0.005   6.123233995736766e-17 1.0 0.0025   -1.0 1.2246467991473532e-16 0.0025'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t4]
    type = ElementGenerator
    input = t3
    nodal_positions = '0 0 0.005   6.123233995736766e-17 1.0 0.0025   -1.0 1.2246467991473532e-16 0.0025   0 0 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t5]
    type = ElementGenerator
    input = t4
    nodal_positions = '0 0 0   6.123233995736766e-17 1.0 0.0025   0 0 -1   -1.0 1.2246467991473532e-16 0.0025'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t6]
    type = ElementGenerator
    input = t5
    nodal_positions = '0 0 0   0 0 0.005   -1.0 1.2246467991473532e-16 0.0025   -1.8369701987210297e-16 -1.0 0.0025'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t7]
    type = ElementGenerator
    input = t6
    nodal_positions = '0 0 0.005   -1.0 1.2246467991473532e-16 0.0025   -1.8369701987210297e-16 -1.0 0.0025   0 0 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t8]
    type = ElementGenerator
    input = t7
    nodal_positions = '0 0 0   -1.0 1.2246467991473532e-16 0.0025   0 0 -1   -1.8369701987210297e-16 -1.0 0.0025'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t9]
    type = ElementGenerator
    input = t8
    nodal_positions = '0 0 0   0 0 0.005   -1.8369701987210297e-16 -1.0 0.0025   1.0 0.0 0.0025'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t10]
    type = ElementGenerator
    input = t9
    nodal_positions = '0 0 0.005   -1.8369701987210297e-16 -1.0 0.0025   1.0 0.0 0.0025   0 0 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t11]
    type = ElementGenerator
    input = t10
    nodal_positions = '0 0 0   -1.8369701987210297e-16 -1.0 0.0025   0 0 -1   1.0 0.0 0.0025'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [repair]
    type = MeshRepairGenerator
    input = t11
    fix_node_overlap = true
    fix_sliver_elements = true
  []
  [diag]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_non_conformality = ERROR
    examine_element_overlap = ERROR
    examine_element_volumes = ERROR
  []
[]
[Problem]
  solve=false
[]
[Executioner]
  type=Steady
[]
