[Mesh]
  file = square_nodes.e
  uniform_refine = 0
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
  [./bott_left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
  [./bott_right]
    type = DirichletBC
    variable = v
    boundary = 2
    value = 1
  [../]
  [./up_right]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]
  [./up_left]
    type = DirichletBC
    variable = v
    boundary = 4
    value = 1
  [../]
  [./the_rest_u]
    type = DirichletBC
    variable = u
    boundary = '5 6 7 8'
    value = 0
  [../]
  [./the_rest_v]
    type = DirichletBC
    variable = v
    boundary = '5 6 7 8'
    value = 0
  [../]
[]

[UserObjects]
  [./bubbles]
    use_single_map = false
    type = FeatureFloodCount
    variable = 'u v'
    threshold = 0.3
    execute_on = timestep_end
    outputs = none
    flood_entity_type = NODAL
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
