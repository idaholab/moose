# Verify integrated inactive-law semantics and the backward-compatible gauge_stress diagnostic.

!include lps_multi_equivalence.i

[Materials/lps]
  coefficient := 'coef_zero coef_linear'
  power := '3 1'
[]

[Postprocessors]
  [legacy_gauge]
    type = ADElementAverageMaterialProperty
    mat_prop = gauge_stress
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [inactive_gauge]
    type = ADElementAverageMaterialProperty
    mat_prop = gauge_stress_0
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [active_gauge]
    type = ADElementAverageMaterialProperty
    mat_prop = gauge_stress_1
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[UserObjects]
  [check_active_gauge]
    type = Terminator
    expression = 'active_gauge <= 0'
    execute_on = FINAL
    error_level = ERROR
    message = 'The active second LPS mechanism did not report a positive gauge stress.'
  []
  [check_inactive_gauge]
    type = Terminator
    expression = 'abs(inactive_gauge) > 1e-12'
    execute_on = FINAL
    error_level = ERROR
    message = 'The exactly inactive leading LPS mechanism reported a nonzero gauge stress.'
  []
  [check_legacy_gauge]
    type = Terminator
    expression = 'abs(legacy_gauge - active_gauge) / (abs(active_gauge) + 1) > 1e-12'
    execute_on = FINAL
    error_level = ERROR
    message = 'The legacy gauge_stress property did not follow the first active LPS mechanism.'
  []
[]

[Outputs]
  csv := false
[]
