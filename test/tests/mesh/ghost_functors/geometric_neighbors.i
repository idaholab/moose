[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8

  # We are testing geometric ghosted functors
  # so we have to use distributed mesh
  parallel_type = distributed

  # We are testing these two parameters
  num_ghosted_layers = 1
  ghost_point_neighbors = false
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./ghosted_elements]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./proc]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./random_elemental]
    type = GhostAux
    variable = ghosted_elements
    ghost_user_object = ghost_uo
    execute_on = initial

  [../]
  [./proc ]
    type = ProcessorIDAux
    variable = proc
    execute_on = initial
  [../]
[]

[UserObjects]
  [./ghost_uo]
    type = GhostUserObject
    execute_on = initial
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]
