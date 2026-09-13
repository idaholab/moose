# Two HEX8 sliver columns stacked end to end, sharing their small cross-section face. Each is thin
# in two dimensions. Collapsing either onto its long axis would collapse the shared cross-section
# the other depends on, degenerating it, so the repair conservatively leaves both in place and
# reports them (a degenerate cluster cannot be resolved one column at a time).
[Mesh]
  [column_lower]
    type = ElementGenerator
    nodal_positions = '0 0 0   1e-3 0 0   1e-3 1e-3 0   0 1e-3 0   0 0 1   1e-3 0 1   1e-3 1e-3 1   0 1e-3 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [column_upper]
    type = ElementGenerator
    input = column_lower
    nodal_positions = '0 0 1   1e-3 0 1   1e-3 1e-3 1   0 1e-3 1   0 0 2   1e-3 0 2   1e-3 1e-3 2   0 1e-3 2'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [repair]
    type = MeshRepairGenerator
    input = column_upper
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
