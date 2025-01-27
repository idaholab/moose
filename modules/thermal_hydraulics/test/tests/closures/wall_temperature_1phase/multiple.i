!include base.i

[Materials]
  [f_Hw_mat]
    type = ADGenericConstantMaterial
    block = 'pipe'
    prop_names = 'f_D Hw:1 Hw:2'
    prop_values = '0 100 100'
  []
[]

[Components]
  [ht2]
    type = HeatTransferFromSpecifiedTemperature1Phase
    flow_channel = pipe
    T_wall = 700
  []
[]
