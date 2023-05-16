[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
[]

[MultiApps]
  [sub]
    type = CentroidMultiApp
    input_files = 'recursive.i'
  []
[]
