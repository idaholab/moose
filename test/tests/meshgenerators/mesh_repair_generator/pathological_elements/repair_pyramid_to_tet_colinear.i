# A PYRAMID5 whose base vertex 3 (1,1,0) lies on the diagonal between its base neighbors (2,2,0)
# and (0,0,0) - a colinear "not sticking out" base vertex. Removing it reduces the quad base to a
# triangle, turning the pyramid into a tetrahedron (PYRAMID5 -> TET4). The pyramid is isolated, so
# the colinear vertex is redundant everywhere and the reduction is conformal.
[Mesh]
  [pyramid_colinear]
    type = ElementGenerator
    nodal_positions = '0 0 0   2 0 0   2 2 0   1 1 0   1 0.5 1'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [repair]
    type = MeshRepairGenerator
    input = pyramid_colinear
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
