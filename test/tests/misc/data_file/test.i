[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  [data_file]
    type = DataFileNameTest
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]
