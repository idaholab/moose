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
[Postprocessors]
  [T_wall_1]
    type = ADElementAverageMaterialProperty
    mat_prop = T_wall:1
    execute_on = 'INITIAL'
  []
  [T_wall_2]
    type = ADElementAverageMaterialProperty
    mat_prop = T_wall:2
    execute_on = 'INITIAL'
  []
[]
