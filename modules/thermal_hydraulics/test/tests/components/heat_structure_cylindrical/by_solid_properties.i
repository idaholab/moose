# Tests that cylindrical heat structure thermal properties can be defined in the SolidProperties block

!include part_base.i

[SolidProperties]
  [fuel_sp]
    type = ThermalFunctionSolidProperties
    rho = 1.0412e2
    cp = 288.734
    k = 3.65
  []
  [gap_sp]
    type = ThermalFunctionSolidProperties
    rho = 1.0
    cp = 1.0
    k = 1.084498
  []
  [clad_sp]
    type = ThermalFunctionSolidProperties
    rho = 6.6e1
    cp = 321.384
    k = 16.48672
  []
[]

[Components]
  [hs]
    solid_properties = 'fuel_sp gap_sp clad_sp'
    solid_properties_T_ref = '300 300 300'
  []
[]
