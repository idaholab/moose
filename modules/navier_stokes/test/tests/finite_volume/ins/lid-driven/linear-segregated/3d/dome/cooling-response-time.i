T_heat_trigger = 0
# Keep active cooling disabled so this run measures cooldown from the restart state.
T_cooling_trigger = ${units 100 degC -> K}
T_cool_terminate = ${units 60 degF -> K}

!include dome.i

[Problem]
  restart_file_base = heating-response-time_cp/LATEST
[]

[UserObjects]
  [cooling_complete]
    type = Terminator
    expression = 'sensor_avg <= ${T_cool_terminate}'
    fail_mode = HARD
    error_level = INFO
    message = 'Cooling response complete: six-sensor average reached the 60 F trigger point.'
    execute_on = TIMESTEP_END
  []
[]
