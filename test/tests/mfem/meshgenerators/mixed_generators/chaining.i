[Mesh]
  [gen]
    type = MFEMGeneratedMeshGenerator
    dim = 1
    nx = 1
  []
  [rename]
    type = RenameBlockGenerator
    input = gen
  []
[]
