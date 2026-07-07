# A flat HEX8 whose top and bottom faces are the quad bases of two PYRAMID5s. Collapsing the
# squashed pair merges the two pyramids base to base; this exercises the hex collapse with non-hex
# cap neighbors.
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
    nodal_positions = '0 0 0  0 1 0  1 1 0  1 0 0  0.5 0.5 -1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [above]
    type = ElementGenerator
    input = below
    nodal_positions = '0 0 0.01  1 0 0.01  1 1 0.01  0 1 0.01  0.5 0.5 1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
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
