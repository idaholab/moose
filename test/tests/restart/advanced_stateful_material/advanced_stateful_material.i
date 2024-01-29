[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 5
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Materials]
  [test]
    type = RestartStatefulMaterial
    real_names = 'a b'
    real_values = '1 2'
    real_stateful_names = 'a b'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  checkpoint = true
[]
