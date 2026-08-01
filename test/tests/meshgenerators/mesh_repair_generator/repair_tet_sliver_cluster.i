# A cluster of needle slivers sharing a short interior edge, surrounded above and below by more
# tets (valence-8 interior node). Exercises the node-disjoint multi-pass collapse.
[Mesh]
  [t0]
    type = ElementGenerator
    nodal_positions = '0 0 -0.0025   0 0 0.0025   1.0 0.0 0   6.123233995736766e-17 1.0 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t1]
    type = ElementGenerator
    input = t0
    nodal_positions = '0 0 -0.0025   1.0 0.0 0   0 0 -1   6.123233995736766e-17 1.0 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t2]
    type = ElementGenerator
    input = t1
    nodal_positions = '0 0 0.0025   1.0 0.0 0   6.123233995736766e-17 1.0 0   0 0 0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t3]
    type = ElementGenerator
    input = t2
    nodal_positions = '0 0 0.5   1.0 0.0 0   6.123233995736766e-17 1.0 0   0 0 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t4]
    type = ElementGenerator
    input = t3
    nodal_positions = '0 0 -0.0025   0 0 0.0025   6.123233995736766e-17 1.0 0   -1.0 1.2246467991473532e-16 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t5]
    type = ElementGenerator
    input = t4
    nodal_positions = '0 0 -0.0025   6.123233995736766e-17 1.0 0   0 0 -1   -1.0 1.2246467991473532e-16 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t6]
    type = ElementGenerator
    input = t5
    nodal_positions = '0 0 0.0025   6.123233995736766e-17 1.0 0   -1.0 1.2246467991473532e-16 0   0 0 0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t7]
    type = ElementGenerator
    input = t6
    nodal_positions = '0 0 0.5   6.123233995736766e-17 1.0 0   -1.0 1.2246467991473532e-16 0   0 0 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t8]
    type = ElementGenerator
    input = t7
    nodal_positions = '0 0 -0.0025   0 0 0.0025   -1.0 1.2246467991473532e-16 0   -1.8369701987210297e-16 -1.0 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t9]
    type = ElementGenerator
    input = t8
    nodal_positions = '0 0 -0.0025   -1.0 1.2246467991473532e-16 0   0 0 -1   -1.8369701987210297e-16 -1.0 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t10]
    type = ElementGenerator
    input = t9
    nodal_positions = '0 0 0.0025   -1.0 1.2246467991473532e-16 0   -1.8369701987210297e-16 -1.0 0   0 0 0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t11]
    type = ElementGenerator
    input = t10
    nodal_positions = '0 0 0.5   -1.0 1.2246467991473532e-16 0   -1.8369701987210297e-16 -1.0 0   0 0 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t12]
    type = ElementGenerator
    input = t11
    nodal_positions = '0 0 -0.0025   0 0 0.0025   -1.8369701987210297e-16 -1.0 0   1.0 0.0 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t13]
    type = ElementGenerator
    input = t12
    nodal_positions = '0 0 -0.0025   -1.8369701987210297e-16 -1.0 0   0 0 -1   1.0 0.0 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t14]
    type = ElementGenerator
    input = t13
    nodal_positions = '0 0 0.0025   -1.8369701987210297e-16 -1.0 0   1.0 0.0 0   0 0 0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [t15]
    type = ElementGenerator
    input = t14
    nodal_positions = '0 0 0.5   -1.8369701987210297e-16 -1.0 0   1.0 0.0 0   0 0 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [repair]
    type = MeshRepairGenerator
    input = t15
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
