[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./features]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./ghosts]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./halos]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./proc_id]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[ICs]
  [./c]
    type = LatticeSmoothCircleIC
    variable = c
    invalue = 1.0
    outvalue = 0.0001
    circles_per_side = '2 2'
    pos_variation = 10.0
    radius = 8.0
    int_width = 5.0
    radius_variation_type = uniform
    avoid_bounds = false
  [../]
[]

[BCs]
  [./Periodic]
    [./c]
      variable = c
      auto_direction = 'x y'
    [../]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = c
  [../]
[]

[AuxKernels]
  [./features]
    type = FeatureFloodCountAux
    variable = features
    execute_on = 'initial timestep_end'
    flood_counter = features
  [../]
  [./ghosts]
    type = FeatureFloodCountAux
    variable = ghosts
    field_display = GHOSTED_ENTITIES
    execute_on = 'initial timestep_end'
    flood_counter = features
  [../]
  [./halos]
    type = FeatureFloodCountAux
    variable = halos
    field_display = HALOS
    execute_on = 'initial timestep_end'
    flood_counter = features
  [../]
  [./proc_id]
    type = ProcessorIDAux
    variable = proc_id
    execute_on = 'initial timestep_end'
  [../]
[]

[Postprocessors]
  [./features]
    type = FeatureFloodCount
    variable = c
    flood_entity_type = ELEMENTAL
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
  exodus = true
[]
