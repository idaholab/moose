!include part_base.i

[Materials]
  [hs-mat]
    type = ADGenericConstantMaterial
    block = hs:blk
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '1 1 1'
  []
[]
