# Tests that a plate heat structure ActionComponent generates the expected multi-region 2D mesh
# (two named transverse regions, stacked and stitched via MeshGenerators) with the same boundary
# naming convention as classic HeatStructurePlate.

[ActionComponents]
  [hs]
    type = HeatStructurePlate
    position = '0 0 0'
    orientation = '1 0 0'

    length = 1.0
    n_elems = 5
    depth = 0.1

    names = 'A B'
    widths = '0.01 0.02'
    n_part_elems = '2 3'

    initial_T = 300
  []
[]

[Outputs]
  exodus = true
[]
