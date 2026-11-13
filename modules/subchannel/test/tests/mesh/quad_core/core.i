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
    # Ducts only need to not overlap. Every duct here fits in a 0.5 m square, so any pitch of at
    # least 0.5 m works for any pattern; this pattern first overlaps below 0.425 m.
    assembly_pitch = 0.6
    pattern = '0 1  0;
               1 0 -1'
  []
[]
