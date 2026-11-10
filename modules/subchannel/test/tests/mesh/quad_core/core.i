# Three-by-two square core with one empty position: ducted 4x3 and 3x2 subchannel assemblies
# Outer duct sizes, (n - 1) * pitch + 2 * side_gap:
#   big:   0.49 m x 0.36 m
#   small: 0.36 m x 0.23 m
[GlobalParams]
  n_cells = 2
  heated_length = 1.0
  pitch = 0.13
  side_gap = 0.05
[]

[QuadSubChannelMesh]
  [big]
    type = SCMQuadAssemblyMeshGenerator
    nx = 4
    ny = 3
    pin_diameter = 0.1
  []
  [big_duct]
    type = SCMQuadDuctMeshGenerator
    input = big
    nx = 4
    ny = 3
  []
  [small]
    type = SCMQuadAssemblyMeshGenerator
    nx = 3
    ny = 2
    pin_diameter = 0.1
  []
  [small_duct]
    type = SCMQuadDuctMeshGenerator
    input = small
    nx = 3
    ny = 2
  []
  [core]
    type = SCMQuadCoreMeshGenerator
    inputs = 'big_duct small_duct'
    # Every duct fits in a 0.5 m x 0.5 m cell, so a 0.5 m pitch is the smallest without overlap
    assembly_pitch = 0.6
    pattern = '0 1  0;
               1 0 -1'
  []
[]
