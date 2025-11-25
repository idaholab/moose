p_value = 1e5
T_value = 300

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Postprocessors]
  # inj1
  [p_inj1]
    type = ConstantPostprocessor
    value = ${p_value}
  []
  [T_inj1]
    type = ConstantPostprocessor
    value = ${T_value}
  []
  [mass_rate_inj2]
    type = Receiver
  []
  [energy_rate_inj2]
    type = Receiver
  []

  # inj2
  [p_inj2]
    type = ConstantPostprocessor
    value = ${p_value}
  []
  [T_inj2]
    type = ConstantPostprocessor
    value = ${T_value}
  []
  [mass_rate_inj1]
    type = Receiver
  []
  [energy_rate_inj1]
    type = Receiver
  []

  # pro1
  [p_pro1]
    type = ConstantPostprocessor
    value = ${p_value}
  []
  [T_pro1]
    type = ConstantPostprocessor
    value = ${T_value}
  []
  [mass_rate_pro2]
    type = Receiver
  []
  [energy_rate_pro2]
    type = Receiver
  []

  # pro2
  [p_pro2]
    type = ConstantPostprocessor
    value = ${p_value}
  []
  [T_pro2]
    type = ConstantPostprocessor
    value = ${T_value}
  []
  [mass_rate_pro1]
    type = Receiver
  []
  [energy_rate_pro1]
    type = Receiver
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  dt = 1
[]
