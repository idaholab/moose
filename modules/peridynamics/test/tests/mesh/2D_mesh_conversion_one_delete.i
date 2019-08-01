[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = 2D_2blocks.e
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = fmg
    retain_fe_mesh = false
    convert_block_ids = 1
  [../]
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
