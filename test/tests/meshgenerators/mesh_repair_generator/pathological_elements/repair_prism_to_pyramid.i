# A PRISM6 (wedge) pinched at one corner: its vertical edge 0-3 is short (top node 3 within 1e-7 of
# bottom node 0). Collapsing that vertical edge turns the wedge into a pyramid (PRISM6 -> PYRAMID5):
# the merged corner becomes the apex and the opposite lateral quad becomes the base. The wedge is
# isolated, so the collapse is local.
[Mesh]
  [prism_pinched]
    type = ElementGenerator
    nodal_positions = '0 0 0   1 0 0   0 1 0   0 0 1e-7   1 0 1   0 1 1'
    element_connectivity = '0 1 2 3 4 5'
    elem_type = PRISM6
  []
  [repair]
    type = MeshRepairGenerator
    input = prism_pinched
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
