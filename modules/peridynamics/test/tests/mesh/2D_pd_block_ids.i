# Run with --mesh-only

[Mesh]
  active = 'fe_mesh pd_mesh'
  type = PeridynamicsMesh
  horizon_number = 2
  [fe_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 18
    ny = 9
    subdomain_ids = '0 0 0 1 1 1 3 3 3 4 4 4 2 2 2 5 5 5
                     0 0 0 1 1 1 3 3 3 4 4 4 2 2 2 5 5 5
                     0 0 0 1 1 1 3 3 3 4 4 4 2 2 2 5 5 5
                     0 0 0 1 1 1 3 3 3 4 4 4 2 2 2 5 5 5
                     0 0 0 1 1 1 3 3 3 4 4 4 2 2 2 5 5 5
                     0 0 0 1 1 1 3 3 3 4 4 4 2 2 2 5 5 5
                     0 0 0 1 1 1 3 3 3 4 4 4 2 2 2 5 5 5
                     0 0 0 1 1 1 3 3 3 4 4 4 2 2 2 5 5 5
                     0 0 0 1 1 1 3 3 3 4 4 4 2 2 2 5 5 5'
  []
  [pd_mesh]
    type = MeshGeneratorPD
    input = fe_mesh
    retain_fe_mesh = false
    bonding_block_pairs = '3 4; 2 5'
    merge_pd_blocks = false
    merge_pd_interfacial_blocks = false
  []
[]
