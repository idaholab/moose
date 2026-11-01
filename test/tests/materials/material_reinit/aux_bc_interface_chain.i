# A boundary-restricted elemental auxiliary kernel reads an interface material property, which reads
# a block property on both sides of the interface, which in turn reads another block property. All
# producers on both sides must be computed.
[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    xmax = 2
    ny = 2
    ymax = 2
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '1 0 0'
    top_right = '2 2 0'
    block_id = 1
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = block1
    primary_block = 0
    paired_block = 1
    new_boundary = interface
  []
[]

[AuxVariables]
  [jump]
    order = CONSTANT
    family = MONOMIAL
    block = 0
  []
[]

[AuxKernels]
  [jump]
    type = ADMaterialRealAux
    variable = jump
    property = jump
    boundary = interface
  []
[]

[Materials]
  # property = 1 + A is 1 + 5 = 6 on block 0 and 1 + 2 = 3 on block 1, so jump = 3
  [A_block0]
    type = ADMaterialReinitTest
    block = 0
    property = A
    value = 5
  []
  [A_block1]
    type = ADMaterialReinitTest
    block = 1
    property = A
    value = 2
  []
  [property]
    type = ADMaterialReinitTest
    property = property
    value = 1
    coupled_property = A
  []
  [jump]
    type = PropertyJumpInterfaceMaterial
    property = property
    boundary = interface
  []
[]

[Postprocessors]
  [jump]
    type = ElementAverageValue
    variable = jump
    block = 0
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
