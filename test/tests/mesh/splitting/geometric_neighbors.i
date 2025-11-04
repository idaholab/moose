[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 2

  xmax = 8
  ymax = 2

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
    element_user_object = ghost_uo
    field_name = "ghosted"
    execute_on = initial
  [../]
  [./proc]
    type = ProcessorIDAux
    variable = proc
    execute_on = initial
  [../]
[]

[UserObjects]
  [./ghost_uo]
    type = ElemSideNeighborLayersGeomTester
    execute_on = initial
    element_side_neighbor_layers = 2
  [../]
[]

[Postprocessors]
  [./num_elems]
    type = NumElements
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
