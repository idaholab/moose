# This input file is used to test ThermalSolidPropertiesPostprocessor.

T_ref = 1000.0

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[SolidProperties]
  [solid_props]
    type = ThermalSS316Properties
  []
[]

[Postprocessors]
  [density]
    type = ThermalSolidPropertiesPostprocessor
    solid_properties = solid_props
    property = density
    T = ${T_ref}
    execute_on = 'INITIAL'
  []
  [specific_heat]
    type = ThermalSolidPropertiesPostprocessor
    solid_properties = solid_props
    property = specific_heat
    T = ${T_ref}
    execute_on = 'INITIAL'
  []
  [thermal_conductivity]
    type = ThermalSolidPropertiesPostprocessor
    solid_properties = solid_props
    property = thermal_conductivity
    T = ${T_ref}
    execute_on = 'INITIAL'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = 'thermal_solid_properties_postprocessor'
  [csv]
    type = CSV
    execute_on = 'INITIAL'
  []
[]
