# A single flat TET4 sliver standing alone: all four of its nodes are on the mesh boundary, so every
# edge collapse would distort the boundary surface and is rejected. The repair therefore leaves the
# sliver in place and reports it as skipped, rather than corrupting the mesh. (A sliver can only be
# collapsed when it has an interior node to remove, as in repair_tet_sliver.i.)
[Mesh]
  [tet0]
    type = ElementGenerator
    nodal_positions = '0 0 0   1 0 0   0.5 0.866 0   0.5 0.289 0.01'
    element_connectivity = '0 1 2 3'
    elem_type = TET4
  []
  [repair]
    type = MeshRepairGenerator
    input = tet0
    fix_sliver_elements = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
