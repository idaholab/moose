# A circle with a single ring of elements, without the enclosing square. The outer
# boundary is formed by the elements bridging the inner square to the circle.
[Mesh]
  [ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '1'
    rings = '1'
    has_outer_square = false
    preserve_volumes = true
    boundary_name_prefix = 'ccmg_'
  []
[]
