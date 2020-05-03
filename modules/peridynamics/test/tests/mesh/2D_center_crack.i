# Test for generated mesh with predefined center crack geometry

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3
  cracks_start = '0.25 0.5 0'
  cracks_end = '0.75 0.5 0'

  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 50
    ny = 50
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
