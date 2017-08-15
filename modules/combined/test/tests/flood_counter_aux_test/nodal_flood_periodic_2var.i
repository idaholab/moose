[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  nz = 0

  xmax = 40
  ymax = 40
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./bubble_map0]
    order = FIRST
    family = LAGRANGE
  [../]

  [./bubble_map1]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./diffv]
    type = Diffusion
    variable = v
  [../]

  [./forcing_1]
    type = GaussContForcing
    variable = u
    x_center = 1.0
    y_center = 1.0
    x_spread = 0.5
    y_spread = 0.5
  [../]

  [./forcing_2]
    type = GaussContForcing
    variable = u
    x_center = 20.0
    y_center = 39.0
    x_spread = 0.5
    y_spread = 0.5
  [../]

  [./forcing_3]
    type = GaussContForcing
    variable = u
    x_center = 39.0
    y_center = 20.0
    x_spread = 0.5
    y_spread = 0.5
  [../]

  [./forcing_4]
    type = GaussContForcing
    variable = u
    x_center = 15.0
    y_center = 15.0
    x_spread = 0.5
    y_spread = 0.5
  [../]

  [./forcing_1v]
    type = GaussContForcing
    variable = v
    x_center = 8.0
    y_center = 8.0
    x_spread = 0.5
    y_spread = 0.5
  [../]

  [./forcing_2v]
    type = GaussContForcing
    variable = v
    x_center = 18.0
    y_center = 22.0
    x_spread = 0.5
    y_spread = 0.5
  [../]

  [./forcing_3v]
    type = GaussContForcing
    variable = v
    x_center = 39.0
    y_center = 20.0
    x_spread = 0.5
    y_spread = 0.5
  [../]

  [./forcing_4v]
    type = GaussContForcing
    variable = v
    x_center = 32.0
    y_center = 8.0
    x_spread = 0.5
    y_spread = 0.5
  [../]

  [./dot]
    type = TimeDerivative
    variable = u
  [../]

  [./dotv]
    type = TimeDerivative
    variable = v
  [../]
[]

[AuxKernels]
  [./mapper0]
    type = FeatureFloodCountAux
    variable = bubble_map0
    execute_on = timestep_end
    flood_counter = bubbles
    map_index = 0
  [../]

  [./mapper1]
    type = FeatureFloodCountAux
    variable = bubble_map1
    execute_on = timestep_end
    flood_counter = bubbles
    map_index = 1
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      variable = 'u v'
      auto_direction = 'x y'
    [../]
  [../]
[]

[UserObjects]
  [./bubbles]
    type = FeatureFloodCount
    variable = 'u v'
    threshold = 0.3
    execute_on = timestep_end
    use_single_map = false
    use_global_numbering = true
    outputs = none
    flood_entity_type = NODAL
  [../]
[]

[Executioner]
  type = Transient
  dt = 4.0
  num_steps = 5
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out_2var
  exodus = true
[]
