T_heat_trigger = ${units 68 degF -> K}
# Keep active cooling disabled during the heating study.
T_cooling_trigger = ${units 100 degC -> K}

!include dome.i

[Problem]
  restart_file_base = test_cp/LATEST
[]

[UserObjects]
  [heating_complete]
    type = Terminator
    expression = 'heating_mode < 0.5'
    fail_mode = HARD
    error_level = INFO
    message = 'Heating response complete: six-sensor average reached the 68 F set point.'
    execute_on = TIMESTEP_END
  []
[]

[Outputs]
  file_base = heating-response-time
[]
