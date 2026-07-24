# A flat PYRAMID5 sliver (apex just above its quad base) standing alone: its quad base is a free
# boundary face with no element across it, so there is nothing to absorb the pyramid into. The
# repair leaves the sliver in place and reports it as skipped, rather than corrupting the mesh.
# (A sliver pyramid can only be absorbed when an element shares its quad base, as in
# repair_pyramid_sliver.i.)
[Mesh]
  [pyr]
    type = ElementGenerator
    nodal_positions = '0 0 1  1 0 1  1 1 1  0 1 1  0.5 0.5 1.01'
    element_connectivity = '0 1 2 3 4'
    elem_type = PYRAMID5
  []
  [repair]
    type = MeshRepairGenerator
    input = pyr
    fix_sliver_elements = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
