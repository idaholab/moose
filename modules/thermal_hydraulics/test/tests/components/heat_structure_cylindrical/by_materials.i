# Tests that cylindrical heat structure thermal properties can be defined in the Materials block

!include part_base.i

[SolidProperties]
  [function_sp]
    type = ThermalFunctionSolidProperties
    rho = 6.6e1
    cp = 321.384
    k = 16.48672
  []
[]

[Materials]
  [fuel-mat]
    type = ADGenericConstantMaterial
    block = hs:FUEL
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '3.65 288.734 1.0412e2'
  []

  [gap-mat]
    type = ADGenericConstantMaterial
    block = hs:GAP
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '1.084498 1.0 1.0'
  []

  [clad-mat]
    type = ADConstantDensityThermalSolidPropertiesMaterial
    block = hs:CLAD
    sp = function_sp
    temperature = T_solid
    T_ref = 300
  []
[]
