# Verify that the generic adaptive-substep history is based only on the previous accepted global
# step. The first timestep is accepted with dt = 1. IterationAdaptiveDT then grows to dt = 2, where
# the Terminator intentionally soft-fails the timestep. The retry at dt = 1 must retain the same
# previous accepted controller rate while its predicted increment scales with the current dt.

!include lps_adaptive_substepping.i

[UserObjects]
  [force_history_cutback]
    type = Terminator
    expression = 'dt > 1.5'
    fail_mode = SOFT
    error_level = INFO
    message = 'Intentional adaptive-history rollback test cutback.'
    execute_on = TIMESTEP_END
  []
[]

[Postprocessors]
  [dt]
    type = TimestepSize
  []
[]

[Executioner]
  dt := 1
  end_time := 3
  [TimeStepper]
    type = IterationAdaptiveDT
    dt = 1
    optimal_iterations = 100
    iteration_window = 1
    growth_factor = 2
    cutback_factor_at_failure = 0.5
  []
[]
