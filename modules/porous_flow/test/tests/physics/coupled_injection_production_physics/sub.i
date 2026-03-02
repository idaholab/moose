e_source = 1e5
mass_rate_inj1 = 0.5
C1_source = 2e-3
energy_rate_inj1 = ${fparse mass_rate_inj1 * e_source}
C1_rate_inj1 = ${fparse mass_rate_inj1 * C1_source}
mass_rate_pro1 = -0.5
energy_rate_pro1 = ${fparse mass_rate_pro1 * e_source}
C1_rate_pro1 = ${fparse mass_rate_pro1 * C1_source}

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Postprocessors]
  [mass_rate_inj1]
    type = ConstantPostprocessor
    value = ${mass_rate_inj1}
  []
  [energy_rate_inj1]
    type = ConstantPostprocessor
    value = ${energy_rate_inj1}
  []
  [C1_rate_inj1]
    type = ConstantPostprocessor
    value = ${C1_rate_inj1}
  []
  [p_inj1]
    type = Receiver
  []
  [T_inj1]
    type = Receiver
  []
  [C1_inj1]
    type = Receiver
  []

  [mass_rate_pro1]
    type = ConstantPostprocessor
    value = ${mass_rate_pro1}
  []
  [energy_rate_pro1]
    type = ConstantPostprocessor
    value = ${energy_rate_pro1}
  []
  [C1_rate_pro1]
    type = ConstantPostprocessor
    value = ${C1_rate_pro1}
  []
  [p_pro1]
    type = Receiver
  []
  [T_pro1]
    type = Receiver
  []
  [C1_pro1]
    type = Receiver
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  end_time = 5.0
  dt = 1.0
[]
