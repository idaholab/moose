[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  nz = 20
[]

[Executioner]
  type = Steady
  solve_type = JFNK
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]

[Testing]
  [./LotsOfDiffusion]
    [./vars]
      number = 2
    [../]
  [../]
[]
