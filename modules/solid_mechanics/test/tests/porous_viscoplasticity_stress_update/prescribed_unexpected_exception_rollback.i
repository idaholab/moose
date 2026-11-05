# Generic transactional rollback fixture for PorousViscoplasticityStressUpdate.
# The SolidMechanicsTestApp-only material injects std::runtime_error at one selected generic
# constitutive stage, verifies caller/material rollback, and rethrows the original exception.

!include prescribed_two_population.i

AD := ''
use_ad := false

[Materials/lps]
  type := PorousViscoplasticityStressUpdateTest
  failure_point = estimation
  use_substepping := INCREMENT_BASED
  adaptive_substepping = true
  maximum_number_substeps = 16
  population_pressures := '0 0'
  population_pressure_derivatives := '0 0 0 0'
[]

[Materials/test_creep_coefficient]
  expression := '2e-10'
[]

[Outputs]
  csv := false
  exodus := false
[]
