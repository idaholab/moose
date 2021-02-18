[Mesh]
  [gmg_quad]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    show_info = true
  []
  [gmg_quad_block1]
    type = ParsedSubdomainMeshGenerator
    input = gmg_quad
    combinatorial_geometry = 'x > 0.5'
    block_name = 'dummy'
    block_id = 1
    show_info = true
  []

  [gmg_tri]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 4
    ny = 4
    elem_type = TRI3
    show_info = true
  []
  [gmg_tri_block2]
    type = ParsedSubdomainMeshGenerator
    input = gmg_tri
    combinatorial_geometry = 'y > 0.5'
    block_id = 2
    block_name = 'dummy2'
    show_info = true
  []
  [gmg_tri_block3]
    type = ParsedSubdomainMeshGenerator
    input = gmg_tri_block2
    combinatorial_geometry = 'y < 0.5'
    block_id = 3
    block_name = 'dummy3'
    show_info = true
  []

  [pmg]
    type = PatternedMeshGenerator
    inputs = 'gmg_quad_block1 gmg_tri_block3'
    pattern = '0 1 0;
               1 1 0'
    show_info = true
  []

  [interior]
    type = ParsedGenerateSideset
    input = pmg
    combinatorial_geometry = 'x > 0.99 & x < 1.01'
    normal = '1 0 0'
    new_sideset_name = interior
    show_info = true
  []
[]
