[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 2
    xmax = 2
    ymax = 1
  []
  [tri]
    type = ElementsToSimplicesConverter
    input = gmg
  []
  [blk_right]
    type = SubdomainBoundingBoxGenerator
    input = tri
    block_id = 2
    block_name = right_block
    bottom_left = '0 0 0'
    top_right = '2 1 0'
  []
  [blk_left]
    type = SubdomainBoundingBoxGenerator
    input = blk_right
    block_id = 1
    block_name = left_block
    bottom_left = '0 0 0'
    top_right = '1.25 1 0'
  []
  [iface]
    type = SideSetsBetweenSubdomainsGenerator
    input = blk_left
    primary_block = left_block
    paired_block = right_block
    new_boundary = block_interface
  []
  [diag_ss]
    type = ParsedGenerateSideset
    input = iface
    combinatorial_geometry = 'x > 1.7 & x < 1.8 & y > 0.2 & y < 0.3'
    included_subdomains = right_block
    normal = '1 0 0'
    normal_tol = 0.4
    new_sideset_name = interior_diagonal
  []
  [origin_ns]
    type = ExtraNodesetGenerator
    input = diag_ss
    new_boundary = origin_node
    coord = '0 0 0'
  []
  [eeid]
    type = ParsedExtraElementIDGenerator
    input = origin_ns
    expression = '1 + floor(4 * x) + 10 * floor(4 * y)'
    extra_elem_integer_name = tri_id
  []
  [to_quad]
    type = TriToQuadGenerator
    input = eeid
    algorithm = RECOMBINE
    tri_subdomain_name = leftover_tris
  []

  # Both the triangulation and the recombination depend on element id numbering
  allow_renumbering = false
[]
