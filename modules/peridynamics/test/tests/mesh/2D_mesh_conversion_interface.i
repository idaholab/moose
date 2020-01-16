# Two of four disconnected FE mesh blocks are converted to PD mesh blocks
# Interfacial bonds are formed between these two blocks, and assigned with a block ID

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./fmg]
    type = FileMeshGenerator
    file = 2D_4blocks.e
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = fmg
    retain_fe_mesh = false
    convert_block_ids = '2 3'
    #non_convert_block_ids = '1 4'
    connect_block_id_pairs = '2 3'
    construct_peridynamics_sideset = true
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
