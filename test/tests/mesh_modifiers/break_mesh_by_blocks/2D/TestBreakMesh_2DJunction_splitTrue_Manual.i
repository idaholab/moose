[Mesh]
  file = 4ElementJunction.e
  parallel_type = REPLICATED
[]

[MeshModifiers]
  [./breakmesh]
    type = BreakMeshByBlockManual_2DJunction
    split_interface = true
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
