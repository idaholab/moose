[Mesh]
  type = GeneratedMesh
  dim = 2 # Problem dimension
  nx = 4
  ny = 4
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    initial_condition = 1.0
  []
[]

[AuxVariables]
  [halos]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [halos]
    type = FeatureFloodCountAux
    variable = halos
    flood_counter = grain_tracker
    field_display = HALOS
    execute_on = 'initial timestep_end'
  []
[]

[UserObjects]
  [grain_tracker]
    type = GrainTracker
    variable = 'u'
    compute_halo_maps = true # For displaying HALO fields
    execute_on = 'initial timestep_end'
    verbosity_level = 3
  []
[]


[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  csv = true
[]

[Problem]
  solve = false
[]
