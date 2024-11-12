[Postprocessors]
  ##### Pressure on Face B at various axial heights
  ##### -4 pitch from heated entrance
  [Pressure_FB1]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 0.0
  []
  ##### -3 pitch from heated entrance
  [Pressure_FB2]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 0.285
  []
  ##### -2 pitch from heated entrance
  [Pressure_FB3]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 0.57
  []
  ##### -1.5 pitch from heated entrance
  [Pressure_FB4]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 0.7125
  []
  ##### -0.5 pitch from heated entrance
  [Pressure_FB5]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 0.9975
  []
  ##### 0 pitch from heated entrance
  [Pressure_FB6]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 1.14
  []
  #### 3 pitch from heated entrance
  [Pressure_FB7]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
  #### 6 pitch from heated entrance
  [Pressure_FB9]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 2.85
  []
  #### 8 pitch from heated entrance
  [Pressure_FB8]
    type = SubChannelPointValue
    variable = P
    index = 108
    execute_on = 'TIMESTEP_END'
    height = 3.42
  []
  # ###### Temperature at 4' and 6' o clock at plane B (SC) ##########
  # ###### Plane B is 4.4167 P above start of heated section
  # ###### Plane B: z = 2.3987595
  [Temp_B_01]
    type = SubChannelPointValue
    variable = T
    index = 14
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_02]
    type = SubChannelPointValue
    variable = T
    index = 28
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_03]
    type = SubChannelPointValue
    variable = T
    index = 45
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_04]
    type = SubChannelPointValue
    variable = T
    index = 47
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_05]
    type = SubChannelPointValue
    variable = T
    index = 33
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_06]
    type = SubChannelPointValue
    variable = T
    index = 23
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_07]
    type = SubChannelPointValue
    variable = T
    index = 39
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_08]
    type = SubChannelPointValue
    variable = T
    index = 62
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_09]
    type = SubChannelPointValue
    variable = T
    index = 86
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_10]
    type = SubChannelPointValue
    variable = T
    index = 66
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_11]
    type = SubChannelPointValue
    variable = T
    index = 51
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_12]
    type = SubChannelPointValue
    variable = T
    index = 77
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_13]
    type = SubChannelPointValue
    variable = T
    index = 79
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_14]
    type = SubChannelPointValue
    variable = T
    index = 106
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_15]
    type = SubChannelPointValue
    variable = T
    index = 112
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_16]
    type = SubChannelPointValue
    variable = T
    index = 111
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_17]
    type = SubChannelPointValue
    variable = T
    index = 117
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_18]
    type = SubChannelPointValue
    variable = T
    index = 119
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_19]
    type = SubChannelPointValue
    variable = T
    index = 120
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_20]
    type = SubChannelPointValue
    variable = T
    index = 95
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_21]
    type = SubChannelPointValue
    variable = T
    index = 54
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_22]
    type = SubChannelPointValue
    variable = T
    index = 56
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  ########### Planar mean of temperature at plane B
  [Temp_B]
    type = SCMPlanarMean
    variable = T
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  ####### Temperature at wall E at plane B
  [Temp_B_E1]
    type = SubChannelPointValue
    variable = T
    index = 121
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_E2]
    type = SubChannelPointValue
    variable = T
    index = 123
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_E4]
    type = SubChannelPointValue
    variable = T
    index = 124
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  [Temp_B_E5]
    type = SubChannelPointValue
    variable = T
    index = 125
    execute_on = 'TIMESTEP_END'
    height = 2.3987595
  []
  ###### Temperature at 8 and 10 'o clock at plane C ##########
  ###### Plane C is 6.75 P above start of heated section
  ###### Plane C: z = 3.06375 [m]
  # [Temp_C_01]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 25
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_02]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 15
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_03]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 18
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_04]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 32
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_05]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 51
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_06]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 53
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_07]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 56
  #   execute_on = 'TIMESTEP_END'
  #   height = 2.3987595
  # []
  # [Temp_C_08]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 29
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_09]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 47
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_10]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 67
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_11]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 93
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_12]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 76
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_13]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 78
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_14]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 79
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_15]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 84
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_16]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 85
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_17]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 116
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_18]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 120
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_19]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 122
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_20]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 97
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_21]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 96
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_22]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 99
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # ########### Planar mean of temperature at plane C
  # [Temp_C]
  #   type = SCMPlanarMean
  #   variable = T
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # ####### Temperature at wall E at plane B
  # [Temp_C_E1]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 121
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_E2]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 123
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_E4]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 124
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  # [Temp_C_E5]
  #   type = SubChannelPointValue
  #   variable = T
  #   index = 125
  #   execute_on = 'TIMESTEP_END'
  #   height = 3.06375
  # []
  ####### Area, wetted-perim, at midpoint of heated section, displacement
  ###### Mid plane is 3P above start of heated section
  ###### Mid plane: z = 7P = 1.995 [m]
  [Area_1]
    type = SubChannelPointValue
    variable = S
    index = 104
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
  [Area_2]
    type = SubChannelPointValue
    variable = S
    index = 105
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
  [Area_3]
    type = SubChannelPointValue
    variable = S
    index = 107
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
  [WP_1]
    type = SubChannelPointValue
    variable = w_perim
    index = 104
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
  [WP_2]
    type = SubChannelPointValue
    variable = w_perim
    index = 105
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
  [WP_3]
    type = SubChannelPointValue
    variable = w_perim
    index = 107
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
  [Disp_1]
    type = SubChannelPointValue
    variable = displacement
    index = 104
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
  [Disp_2]
    type = SubChannelPointValue
    variable = displacement
    index = 105
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
  [Disp_3]
    type = SubChannelPointValue
    variable = displacement
    index = 107
    execute_on = 'TIMESTEP_END'
    height = 1.995
  []
[]
