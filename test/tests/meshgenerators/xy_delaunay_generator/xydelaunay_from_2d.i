[Mesh]
  [outer_bdy]
    # Avoid squares here to make diagonal selection more well-defined
    type = ConcentricCircleMeshGenerator
    num_sectors = 4
    radii = '0.5'
    rings = '3'
    has_outer_square = false
    preserve_volumes = false
  []
  [triang]
    type = XYDelaunayGenerator
    boundary = 'outer_bdy'
    refine_boundary = true
    desired_area = 0.01
  []

  parallel_type = replicated  # for ConcentricCircleMeshGenerator
[]
