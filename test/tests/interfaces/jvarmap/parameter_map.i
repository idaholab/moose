[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Variables]
  [./c0][../]
  [./c1][../]
  [./c2][../]
  [./c3][../]
  [./c4][../]
  [./c5][../]
  [./c6][../]
  [./c7][../]
  [./c8][../]
  [./c9][../]
  [./dummy][../]
[]

[Kernels]
  [./test1]
    type = JvarMapTest
    variable = dummy
    v0 = 'c0 c1 c2 c3 c4'
    v1 = 'c5 c6 c7 c8 c9'
  [../]
  [./test2]
    type = JvarMapTest
    variable = dummy
    v0 = 'c4 c3 c2 c1 c0'
    v1 = 'c9 c8 c7 c6 c5'
  [../]
  [./test3]
    type = JvarMapTest
    variable = dummy
    v0 = 'c4 c8 c2 c6 c0 c5'
    v1 = 'c9 c3 c7 c1'
  [../]
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]
