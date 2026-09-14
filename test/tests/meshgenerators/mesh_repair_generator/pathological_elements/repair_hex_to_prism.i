# A HEX8 with one lateral face pinched to a vertical edge: its front face (nodes 0,1,5,4) has both
# horizontal edges short (node 1 within 1e-7 of node 0, node 5 within 1e-7 of node 4). Collapsing
# those two edges together turns the hex into a prism (HEX8 -> PRISM6): each squashed bottom/top
# face becomes a triangle. The hex is isolated, so the collapse is local.
[Mesh]
  [hex_pinched]
    type = ElementGenerator
    nodal_positions = '0 0 0   1e-7 0 0   1 1 0   0 1 0   0 0 1   1e-7 0 1   1 1 1   0 1 1'
    element_connectivity = '0 1 2 3 4 5 6 7'
    elem_type = HEX8
  []
  [repair]
    type = MeshRepairGenerator
    input = hex_pinched
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
