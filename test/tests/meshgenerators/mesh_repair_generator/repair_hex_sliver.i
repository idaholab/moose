# A flat (z-squashed) HEX8 sliver: its top face is 0.01 above the bottom, sandwiched between two
# HEX8s that share its top and bottom faces, with its four lateral faces on the boundary. The repair
# collapses the squashed pair of opposite faces so the two hexes meet, keeping a valid conformal
# mesh.
[Mesh]
  [flat]
    type = ElementGenerator
    nodal_positions = '0 0 0  1 0 0  1 1 0  0 1 0   0 0 0.01  1 0 0.01  1 1 0.01  0 1 0.01'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [below]
    type = ElementGenerator
    input = flat
    nodal_positions = '0 0 -1  1 0 -1  1 1 -1  0 1 -1   0 0 0  1 0 0  1 1 0  0 1 0'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [above]
    type = ElementGenerator
    input = below
    nodal_positions = '0 0 0.01  1 0 0.01  1 1 0.01  0 1 0.01   0 0 1  1 0 1  1 1 1  0 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [repair]
    type = MeshRepairGenerator
    input = above
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
