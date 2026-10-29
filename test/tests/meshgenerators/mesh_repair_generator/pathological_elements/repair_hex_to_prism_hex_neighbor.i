# A HEX8 pinched at its front face, stacked under a second (healthy, un-pinched) hexahedron that
# shares the pinched hex's top face - and therefore its short top edge. Reducing the pinched hex to
# a prism collapses that shared edge, so the neighbor loses it too; having no standard lower type it
# is rebuilt as a C0Polyhedron (a hexahedron minus one edge). A polyhedron has no reference element,
# so PointLocator-based diagnostics (non-conformality, element overlap) cannot run on a mesh
# containing one; only the element-volume diagnostic is applied here.
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
    fix_pathological_elements = true
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
