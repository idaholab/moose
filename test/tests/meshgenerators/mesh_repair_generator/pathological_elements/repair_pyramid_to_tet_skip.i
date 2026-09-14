# A PYRAMID5 sitting on a HEX8, sharing the hex's top face as its base. That shared face has a short
# edge, so both elements share the collapse edge. Reducing the pyramid to a tetrahedron would force
# the hexahedron to reduce too, but a hex sharing a single collapsed edge has no valid lower type,
# so the repair conservatively leaves the pyramid in place and reports it.
[Mesh]
  [hex]
    type = ElementGenerator
    nodal_positions = '0 0 0   1 0 0   1 1 0   0 1 0   0 0 1   1e-7 0 1   1 1 1   0 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [pyramid]
    type = ElementGenerator
    input = hex
    nodal_positions = '0 0 1   1e-7 0 1   1 1 1   0 1 1   0.5 0.5 2'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [repair]
    type = MeshRepairGenerator
    input = pyramid
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
