!include part_base.i

[SolidProperties]
  [sp]
    type = ThermalFunctionSolidProperties
    rho = 1
    cp = 1
    k = 1
  []
[]

[Components]
  [hs]
    solid_properties = 'sp'
    solid_properties_T_ref = '300'
  []
[]
