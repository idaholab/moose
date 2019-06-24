# Test for generated mesh with predefined double_edged crack geometry

[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 100
  [../]
  [./gpd]
    type = MeshGeneratorPD
    input = gmg
    retain_fe_mesh = false
  [../]
[]

[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3
  cracks_start = '0 0.5 0 1.0 0.5 0'
  cracks_end = '0.25 0.5 0 0.75 0.5 0'
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
