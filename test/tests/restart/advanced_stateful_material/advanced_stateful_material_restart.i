[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 1
  nx = 5
[]

[Problem]
  kernel_coverage_check = false
  restart_file_base = advanced_stateful_material_out_cp/LATEST
  solve = false
[]

[Materials]
  active = 'test'
  [test]
    type = RestartStatefulMaterial
    real_names = 'a b'
    real_stateful_names = 'a b'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  start_time = 1
[]
