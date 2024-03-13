!include advanced_stateful_material_base.i

[Problem]
  restart_file_base = advanced_stateful_material_out_cp/LATEST
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
  start_time = 1
[]
