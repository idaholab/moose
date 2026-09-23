# Both independent pore populations start on explicit generic floors. Unequal uniaxial compression
# keeps a deviatoric driving stress while the local active set prevents volumetric pore collapse.

!include prescribed_two_population.i

# Reuse creep.i's pull function so its root-level strain substitution remains consumed.
strain := -5e-5

[Materials/lps]
  initial_population_0_fraction := 0.5
  # Use zero pore pressure so the imposed compressive matrix stress drives unconstrained
  # population collapse; the active set must then hold both populations at their floors.
  population_pressures := '0 0'
  population_pressure_derivatives := '0 0 0 0'
  population_porosity_floors := '0.05 0.05'
[]


[UserObjects]
  [check_population_floors]
    type = Terminator
    expression = 'abs(population_0_porosity - 0.05) > 1e-9 | abs(population_1_porosity - 0.05) > 1e-9'
    execute_on = FINAL
    fail_mode = HARD
    error_level = ERROR
    message = 'The generic independent two-population solve did not retain both active porosity floors.'
  []
  [check_population_sum]
    type = Terminator
    expression = 'abs(population_0_porosity + population_1_porosity - porosity) > 1e-9'
    execute_on = FINAL
    fail_mode = HARD
    error_level = ERROR
    message = 'The accepted independent pore populations do not sum to total porosity.'
  []
  [check_deviatoric_response]
    type = Terminator
    expression = 'eff_creep_strain != eff_creep_strain | abs(eff_creep_strain) < 1e-10'
    execute_on = FINAL
    fail_mode = HARD
    error_level = ERROR
    message = 'The two-floor regression did not retain a finite nonzero deviatoric viscoplastic response.'
  []
[]
