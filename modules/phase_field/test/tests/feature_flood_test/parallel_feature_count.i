[Mesh]
  type = ImageMesh
  dim = 2
  file = spiral_16x16.png
  scale_to_one = false
[]

[Variables]
  [./u]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxVariables]
  [./feature]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./proc_id]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./feature_ghost]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./nodal_flood_aux]
    type = FeatureFloodCountAux
    variable = feature
    flood_counter = flood_count_pp
    execute_on = 'initial timestep_end'
  [../]
  [./proc_id]
    type = ProcessorIDAux
    variable = proc_id
    execute_on = 'initial timestep_end'
  [../]
  [./ghost]
    type = FeatureFloodCountAux
    variable = feature_ghost
    field_display = GHOSTED_ENTITIES
    flood_counter = flood_count_pp
    execute_on = 'initial timestep_end'
  [../]
[]

[Functions]
  [./tif]
    type = ImageFunction
    component = 0
  [../]
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    function = tif
    variable = u
  [../]
[]

[Postprocessors]
  [./flood_count_pp]
    type = FeatureFloodCount
    variable = u
    threshold = 1.0
    execute_on = 'initial timestep_end'
  [../]
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
