# A PRISM6 pinched along its vertical edge 0-3, sharing that edge with a HEX8. Collapsing the edge
# reduces the wedge to a PYRAMID5 and forces the hexahedron sharing the collapsed edge to lose it
# too; having no standard lower type, the hex is rebuilt as a C0Polyhedron (a hexahedron minus one
# edge) so the collapse proceeds while keeping the region filled. A polyhedron has no reference
# element, so PointLocator-based diagnostics (non-conformality, element overlap) cannot run on a
# mesh containing one; only the element-volume diagnostic is applied here.
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
  [diagnostics]
    type = MeshDiagnosticsGenerator
    input = repair
    examine_element_volumes = ERROR
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
