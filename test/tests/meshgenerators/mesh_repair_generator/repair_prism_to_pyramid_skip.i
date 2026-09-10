# A PRISM6 pinched along its vertical edge 0-3, sharing that edge with a HEX8. Reducing the wedge to
# a pyramid would force the hexahedron (which shares the pinched vertical edge) to reduce too, but a
# hex sharing a single collapsed edge has no valid lower type, so the repair conservatively leaves
# the wedge in place and reports it.
[Mesh]
  [prism]
    type = ElementGenerator
    nodal_positions = '0 0 0   2 0 0   0 2 0   0 0 1e-7   2 0 1   0 2 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [hex]
    type = ElementGenerator
    input = prism
    nodal_positions = '0 0 0   0 2 0   -2 2 0   -2 0 0   0 0 1e-7   0 2 1   -2 2 1   -2 0 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [repair]
    type = MeshRepairGenerator
    input = hex
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
