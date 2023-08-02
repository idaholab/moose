[Mesh]

  [fmg]
    type = FileMeshGenerator
    file = lightbridge_fuel_mesh.i
  []

  [diag]
    type = MeshRepairGenerator
    input = fmg
  []
[]
