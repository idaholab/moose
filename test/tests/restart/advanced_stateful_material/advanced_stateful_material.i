!include advanced_stateful_material_base.i

[Materials]
  [test]
    type = RestartStatefulMaterial
    real_names = 'a b'
    real_values = '1 2'
    real_stateful_names = 'a b'
  []
[]

[Outputs]
  checkpoint = true
[]
