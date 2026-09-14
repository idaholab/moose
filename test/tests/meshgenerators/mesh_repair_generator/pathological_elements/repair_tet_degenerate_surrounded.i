# Four TET4 fan around a single tiny edge at (0.5,0.5,+/-1e-7), each spanning 90 degrees
# through the diamond nodes; they are needle slivers sharing thin faces. Eight good tets
# above and eight below (per-sector columns) surround them. Collapsing the short edge
# removes the four needles and stitches the good tetrahedra into a conformal mesh.
[Mesh]
  [bad0]
    type = ElementGenerator
    nodal_positions = '0.5 0.5 -1e-07  0.5 0.5 1e-07  0.5 0 0  1 0.5 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [up1_0]
    type = ElementGenerator
    input = bad0
    nodal_positions = '0.5 0.5 1e-07  0.5 0 0  1 0.5 0  0.5 0.5 0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [up2_0]
    type = ElementGenerator
    input = up1_0
    nodal_positions = '0.5 0.5 0.5  0.5 0 0  1 0.5 0  0.5 0.5 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [dn1_0]
    type = ElementGenerator
    input = up2_0
    nodal_positions = '0.5 0.5 -1e-07  0.5 0 0  1 0.5 0  0.5 0.5 -0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [dn2_0]
    type = ElementGenerator
    input = dn1_0
    nodal_positions = '0.5 0.5 -0.5  0.5 0 0  1 0.5 0  0.5 0.5 -1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [bad1]
    type = ElementGenerator
    input = dn2_0
    nodal_positions = '0.5 0.5 -1e-07  0.5 0.5 1e-07  1 0.5 0  0.5 1 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [up1_1]
    type = ElementGenerator
    input = bad1
    nodal_positions = '0.5 0.5 1e-07  1 0.5 0  0.5 1 0  0.5 0.5 0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [up2_1]
    type = ElementGenerator
    input = up1_1
    nodal_positions = '0.5 0.5 0.5  1 0.5 0  0.5 1 0  0.5 0.5 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [dn1_1]
    type = ElementGenerator
    input = up2_1
    nodal_positions = '0.5 0.5 -1e-07  1 0.5 0  0.5 1 0  0.5 0.5 -0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [dn2_1]
    type = ElementGenerator
    input = dn1_1
    nodal_positions = '0.5 0.5 -0.5  1 0.5 0  0.5 1 0  0.5 0.5 -1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [bad2]
    type = ElementGenerator
    input = dn2_1
    nodal_positions = '0.5 0.5 -1e-07  0.5 0.5 1e-07  0.5 1 0  0 0.5 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [up1_2]
    type = ElementGenerator
    input = bad2
    nodal_positions = '0.5 0.5 1e-07  0.5 1 0  0 0.5 0  0.5 0.5 0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [up2_2]
    type = ElementGenerator
    input = up1_2
    nodal_positions = '0.5 0.5 0.5  0.5 1 0  0 0.5 0  0.5 0.5 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [dn1_2]
    type = ElementGenerator
    input = up2_2
    nodal_positions = '0.5 0.5 -1e-07  0.5 1 0  0 0.5 0  0.5 0.5 -0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [dn2_2]
    type = ElementGenerator
    input = dn1_2
    nodal_positions = '0.5 0.5 -0.5  0.5 1 0  0 0.5 0  0.5 0.5 -1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [bad3]
    type = ElementGenerator
    input = dn2_2
    nodal_positions = '0.5 0.5 -1e-07  0.5 0.5 1e-07  0 0.5 0  0.5 0 0'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [up1_3]
    type = ElementGenerator
    input = bad3
    nodal_positions = '0.5 0.5 1e-07  0 0.5 0  0.5 0 0  0.5 0.5 0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [up2_3]
    type = ElementGenerator
    input = up1_3
    nodal_positions = '0.5 0.5 0.5  0 0.5 0  0.5 0 0  0.5 0.5 1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [dn1_3]
    type = ElementGenerator
    input = up2_3
    nodal_positions = '0.5 0.5 -1e-07  0 0.5 0  0.5 0 0  0.5 0.5 -0.5'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [dn2_3]
    type = ElementGenerator
    input = dn1_3
    nodal_positions = '0.5 0.5 -0.5  0 0.5 0  0.5 0 0  0.5 0.5 -1'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [repair]
    type = MeshRepairGenerator
    input = dn2_3
    fix_node_overlap = true
    fix_elements_orientation = true
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
