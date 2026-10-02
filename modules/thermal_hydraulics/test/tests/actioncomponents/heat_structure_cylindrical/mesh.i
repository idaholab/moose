# Tests that a cylindrical heat structure ActionComponent generates the expected multi-region 2D
# mesh (three named radial regions, stacked and stitched via MeshGenerators) with the same boundary
# naming convention as classic HeatStructureCylindrical: '<name>:inner'/'<name>:outer' (aggregate
# transverse faces), '<name>:start'/'<name>:end' (aggregate axial faces), per-region axial faces,
# and interior region-interface boundaries.

[ActionComponents]
  [hs]
    type = HeatStructureCylindrical
    position = '0 0 0'
    orientation = '1 0 0'

    length = 1.0
    n_elems = 5

    names = 'FUEL GAP CLAD'
    widths = '0.0046955 0.0000955 0.000673'
    n_part_elems = '3 1 1'

    initial_T = 564.15
  []
[]

[Outputs]
  exodus = true
[]
