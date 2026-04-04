T_heat_trigger = ${units 68 degF -> K}
# Keep active cooling disabled during the heating study.
T_cooling_trigger = ${units 100 degC -> K}

!include dome.i

[AuxVariables]
  [mu_t]
    initial_condition = '${fparse rho * C_mu * ${k_init}^2 / eps_init}'
  []
  [k_t]
    initial_condition = 1.
  []
[]

[Variables]
  [vel_x]
    initial_condition = 0.0
  []
  [vel_y]
    initial_condition = 0.0
  []
  [vel_z]
    initial_condition = 0.0
  []
  [pressure]
    initial_condition = 0
  []
  [TKE]
    initial_condition = ${k_init}
  []
  [TKED]
    initial_condition = ${eps_init}
  []
  [T_fluid]
    initial_condition = ${T_0}
  []
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
