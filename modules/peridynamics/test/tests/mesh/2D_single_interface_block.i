# Four disconnected FE mesh blocks are converted to PD mesh blocks
# Interfacial bonds are formed between blocks 1 and 2, 2 and 3, 3 and 4
# All four PD mesh blocks are combined into one block
# All interfacial bonds are gathered into one block

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
    blocks_to_pd = '1 2 3 4'
    merge_pd_blocks = true
    bonding_block_pairs = '1 2; 2 3; 3 4'
    merge_pd_interfacial_blocks = true
    construct_pd_sidesets = true
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
