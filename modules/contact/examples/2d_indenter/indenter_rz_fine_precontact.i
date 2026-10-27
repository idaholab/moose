!include indenter_rz_fine.i

[Executioner]
  num_steps = 1
[]

[Postprocessors]
  [max_stress_xx]
    type = ElementExtremeValue
    variable = stress_xx
    block = '1 2'
    value_type = max_abs
  []
  [max_normal_lm]
    type = NodalExtremeValue
    variable = contact_normal_lm
    block = contact_secondary_subdomain
    value_type = max_abs
  []
[]

[Outputs]
  file_base = indenter_rz_fine_precontact
  [out]
    enable = false
  []
[]
