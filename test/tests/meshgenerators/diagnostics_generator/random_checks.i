[Mesh]

  [fmg]
    type = FileMeshGenerator
    file = tetra.e
  []

  [diag]
    type = MeshDiagnosticsGenerator
    input = fmg
  []
[]
