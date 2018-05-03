# Test for generated mesh with predefined double_edged crack geometry

[Mesh]
  type = GeneratedMeshPD
  dim = 2
  nx = 100
  horizon_number = 3
  crack_start = '0 0.5 0 1.0 0.5 0'
  crack_end = '0.25 0.5 0 0.75 0.5 0'
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
