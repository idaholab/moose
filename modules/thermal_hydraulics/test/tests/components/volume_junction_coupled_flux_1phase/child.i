[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Postprocessors]
  [p_test]
    type = ConstantPostprocessor
    value = 0.9e5
  []
  [T_test]
    type = ConstantPostprocessor
    value = 300
  []
  [mass_rate_test]
    type = Receiver
  []
  [energy_rate_test]
    type = Receiver
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  end_time = 0.5
  dt = 0.1
[]

[Outputs]
  csv = true
[]
