# A PYRAMID5 with a short base edge (base vertex 1 within 1e-7 of vertex 0). Collapsing that base
# edge reduces the quad base to a triangle, turning the pyramid into a tetrahedron (PYRAMID5 ->
# TET4). The pyramid is isolated (its collapse edge is not shared), so the reduction is local.
[Mesh]
  [pyramid_short]
    type = ElementGenerator
    nodal_positions = '0 0 0   1e-7 0 0   1 1 0   0 1 0   0.3 0.3 1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [repair]
    type = MeshRepairGenerator
    input = pyramid_short
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
