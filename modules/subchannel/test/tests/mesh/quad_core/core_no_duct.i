# Three-by-two square core of core.i built from assemblies without duct meshes
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
  [small]
    type = SCMQuadAssemblyMeshGenerator
    nx = 3
    ny = 2
    pin_diameter = 0.1
  []
  [core]
    type = SCMQuadCoreMeshGenerator
    inputs = 'big small'
    # Ducts only need to not overlap. Every duct here fits in a 0.5 m square, so any pitch of at
    # least 0.5 m works for any pattern; this pattern first overlaps below 0.425 m.
    assembly_pitch = 0.6
    pattern = '0 1  0;
               1 0 -1'
  []
[]
