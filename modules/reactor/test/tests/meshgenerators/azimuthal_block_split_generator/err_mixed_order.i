[Mesh]
  # The element order check is after the replicated mesh check
  # So we enforce the replicated mesh here
  parallel_type = REPLICATED
  [gmg1]
    type = GeneratedMeshGenerator
    dim = 2
    elem_type = QUAD4
    subdomain_ids = 1
  []
  [gmg2]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 2
    xmax = 3
    elem_type = QUAD8
    subdomain_ids = 2
  []
  [cmbn]
    type = CombinerGenerator
    inputs = 'gmg1 gmg2'
  []
  [metadata]
    type = AddMetaDataGenerator
    input = cmbn
    uint_vector_metadata_names = 'num_sectors_per_side_meta'
    uint_vector_metadata_values = '2 2 2 2'
  []
  [cd_azi_define]
    type = AzimuthalBlockSplitGenerator
    input = metadata
    start_angle = 280
    angle_range = 100
    old_blocks = '10 15 20'
    new_block_ids = '100 150 200'
    new_block_names = 'center_tri_new center_new cd_ring_new'
    preserve_volumes = true
  []
[]
