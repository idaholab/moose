# Generic two-population porous-LPS regression owned by solid_mechanics.
# The test-only material supplies linear pressure closures directly, so this input exercises the
# independent (p, q, f0, f1) mechanics without BISON gas EOS, interconnection, or inventory.

!include creep.i

[Materials]
  inactive = 'creep'

  [stress]
    inelastic_models := lps
  []

  [lps]
    type = ${AD}PorousViscoplasticityStressUpdateTest
    coefficient = test_creep_coefficient
    power = 1
    porosity_name = porosity
    use_prescribed_two_population_state = true
    initial_population_0_fraction = 0.6
    test_initial_total_porosity = 0.1
    population_pressures = '4e6 8e6'
    # Exercise the independent pressure-porosity sensitivity path without tying it to a gas EOS.
    population_pressure_derivatives = '2e7 -1e7 5e6 1.5e7'
    population_porosity_floors = '0 0'
    minimum_porosity = 1e-10
    local_newton_tolerance = 1e-10
    local_newton_stagnation_tolerance = 1e-9
    outputs = all
  []

  [test_creep_coefficient]
    type = ${AD}ParsedMaterial
    property_name = test_creep_coefficient
    expression = '2e-11'
  []
[]

[Postprocessors/eff_creep_strain]
  variable := effective_viscoplasticity
[]

[Postprocessors]
  [population_0_porosity]
    type = ${AD}ElementAverageMaterialProperty
    mat_prop = test_population_0_porosity
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [population_1_porosity]
    type = ${AD}ElementAverageMaterialProperty
    mat_prop = test_population_1_porosity
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [population_0_effective_hydrostatic_stress]
    type = ${AD}ElementAverageMaterialProperty
    mat_prop = test_population_0_effective_hydrostatic_stress
    execute_on = TIMESTEP_END
  []
  [population_1_effective_hydrostatic_stress]
    type = ${AD}ElementAverageMaterialProperty
    mat_prop = test_population_1_effective_hydrostatic_stress
    execute_on = TIMESTEP_END
  []
[]
