[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8

  # We are testing geometric ghosted functors
  # so we have to use distributed mesh
  parallel_type = distributed
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
    type = ElementUOAux
    variable = ghosted_elements
    element_user_object = neighbor_layers_tester
    execute_on = initial

  [../]
  [./proc ]
    type = ProcessorIDAux
    variable = proc
    execute_on = initial
  [../]
[]

[UserObjects]
  [./neighbor_layers_tester]
    type = ElemSideNeighborLayersTester
    execute_on = initial
    element_side_neighbor_layers = 1
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
