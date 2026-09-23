# Exercise the iteration-limited stagnation-acceptance contract on the generic scalar solver.
# The state starts on the scalar floor, may release to the FREE branch, and with one Newton iteration
# must be accepted only through the deliberately looser stagnation tolerance. Reduced-f
# globalization is intentionally excluded by the checker.

!include lps_single_substep_equivalence.i

dt := 0.125
substepping := NONE
adaptive := false
verbose := true

[Materials/lps]
  minimum_porosity := 0.1
  local_newton_tolerance = 1e-12
  local_newton_stagnation_tolerance = 0.5
  local_newton_absolute_stress_tolerance = 1
  local_newton_max_iterations = 1
[]

[Executioner]
  end_time := 0.125
[]

[Outputs]
  csv := false
[]
