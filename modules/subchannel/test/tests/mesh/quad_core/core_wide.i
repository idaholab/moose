# Two wide 8x2 assemblies two lattice positions apart with the position between them empty. Each
# duct is (8 - 1) * 0.13 + 2 * 0.05 = 1.01 m wide, so the ducts overlap across the empty position
# at a 0.5 m pitch (centers 1.0 m apart) although they are not neighbors.
[GlobalParams]
  n_cells = 2
  heated_length = 1.0
  pitch = 0.13
  side_gap = 0.05
[]

[QuadSubChannelMesh]
  [wide]
    type = SCMQuadAssemblyMeshGenerator
    nx = 8
    ny = 2
    pin_diameter = 0.1
  []
  [wide_duct]
    type = SCMQuadDuctMeshGenerator
    input = wide
    nx = 8
    ny = 2
  []
  [core]
    type = SCMQuadCoreMeshGenerator
    inputs = 'wide_duct'
    assembly_pitch = 0.5
    pattern = '0 -1 0'
  []
[]
