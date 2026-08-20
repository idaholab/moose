# This test provides an example of an individual LPS viscoplasticity model

!include lps_single_nonsubstepping.i

[Materials]
  [lps]
    adaptive_substepping = true
    use_substepping = INCREMENT_BASED
    substep_strain_tolerance = 1e-8
    max_inelastic_increment = 1e-4
    maximum_number_substeps = 100
  []
[]
