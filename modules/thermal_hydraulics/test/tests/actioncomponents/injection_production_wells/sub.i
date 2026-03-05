p_value = 1e5
T_value = 300
C1_value = 2e-3
C2_value = 0.3

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
  [C1_inj1]
    type = ConstantPostprocessor
    value = ${C1_value}
  []
  [C2_inj1]
    type = ConstantPostprocessor
    value = ${C2_value}
  []
  [mass_rate_inj1]
    type = Receiver
  []
  [energy_rate_inj1]
    type = Receiver
  []
  [C1_rate_inj1]
    type = Receiver
  []
  [C2_rate_inj1]
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
  [C1_inj2]
    type = ConstantPostprocessor
    value = ${C1_value}
  []
  [C2_inj2]
    type = ConstantPostprocessor
    value = ${C2_value}
  []
  [mass_rate_inj2]
    type = Receiver
  []
  [energy_rate_inj2]
    type = Receiver
  []
  [C1_rate_inj2]
    type = Receiver
  []
  [C2_rate_inj2]
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
  [C1_pro1]
    type = ConstantPostprocessor
    value = ${C1_value}
  []
  [C2_pro1]
    type = ConstantPostprocessor
    value = ${C2_value}
  []
  [mass_rate_pro1]
    type = Receiver
  []
  [energy_rate_pro1]
    type = Receiver
  []
  [C1_rate_pro1]
    type = Receiver
  []
  [C2_rate_pro1]
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
  [C1_pro2]
    type = ConstantPostprocessor
    value = ${C1_value}
  []
  [C2_pro2]
    type = ConstantPostprocessor
    value = ${C2_value}
  []
  [mass_rate_pro2]
    type = Receiver
  []
  [energy_rate_pro2]
    type = Receiver
  []
  [C1_rate_pro2]
    type = Receiver
  []
  [C2_rate_pro2]
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
