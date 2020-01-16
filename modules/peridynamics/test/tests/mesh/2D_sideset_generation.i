[Mesh]
  type = PeridynamicsMesh
  horizon_number = 3

  [./fmg]
    type = FileMeshGenerator
    file = circle.e
  [../]
  [./mgpd]
    type = MeshGeneratorPD
    input = fmg
    retain_fe_mesh = false
    construct_peridynamics_sideset = true
  [../]
[]
