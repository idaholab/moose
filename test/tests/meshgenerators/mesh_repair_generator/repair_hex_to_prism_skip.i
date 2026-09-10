# A HEX8 pinched at its front face, stacked under a second (healthy, un-pinched) hexahedron that
# shares the pinched hex's top face - and therefore its short top edge. Reducing the pinched hex to
# a prism would force that neighbor (which shares the collapsed edge) to change type too, which is
# not handled, so the repair conservatively leaves the pinched hex in place and reports it.
[Mesh]
  [hex_pinched]
    type = ElementGenerator
    nodal_positions = '0 0 0   1e-7 0 0   1 1 0   0 1 0   0 0 1   1e-7 0 1   1 1 1   0 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [hex_above]
    type = ElementGenerator
    input = hex_pinched
    nodal_positions = '0 0 1   1e-7 0 1   1 1 1   0 1 1   0 0 2   1 0 2   1 1 2   0 1 2'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [repair]
    type = MeshRepairGenerator
    input = hex_above
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
