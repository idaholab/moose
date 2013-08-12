[Mesh]
  type = FileMesh
  file = twoblocks.e
  # This MeshModifier currently only works with SerialMesh.
  # For more information, refer to #2129.
  distribution = serial
[]

[MeshModifiers]
  [./block_1]
    type = AddAllSideSetsByNormals
  [../]
[]

# This input file is intended to be run with the "--mesh-only" option so
# no other sections are required
