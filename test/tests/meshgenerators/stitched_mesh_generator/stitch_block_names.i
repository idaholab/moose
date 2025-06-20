[Mesh]
  [mesh1]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    subdomain_ids = '0 0
                     0 0'
  []
  [mesh2]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
    subdomain_ids = '1 1
                     1 1'
  []
  [shift]
    type = TransformGenerator
    input = mesh2
    transform = translate
    vector_value = '1 0 0'
  []
  [give_name]
    type = RenameBlockGenerator
    input = shift
    old_block = 1
    new_block = 'a_nice_name'
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'mesh1 give_name'
    stitch_boundaries_pairs = 'right left'
  []
[]
