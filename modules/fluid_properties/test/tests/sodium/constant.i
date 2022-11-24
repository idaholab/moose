# Test implementation of passing constant thermal conductivity and specific heat values to SodiumProperties

[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[FluidProperties/sodium]
  type = SodiumProperties
  thermal_conductivity = 123
  specific_heat = 456
[]

[Materials]
  [./fp_mat]
    type = SodiumPropertiesMaterial
    temperature = 100
    outputs = all
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Postprocessors]
  [./k_avg]
    type = ElementAverageValue
    variable = k
  [../]
  [./cp_avg]
    type = ElementAverageValue
    variable = cp
  [../]
[]

[Outputs]
  csv = true
[]
