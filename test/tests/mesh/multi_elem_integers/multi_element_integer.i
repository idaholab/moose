[Mesh]
  type = MeshGeneratorMesh
  parallel_type = 'replicated'

  [gmg1]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 5
    ny = 5
    extra_element_integers = 'material_id'
  []
  [gmg2]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 1
    xmax = 2
    ymin = 0
    ymax = 1
    nx = 5
    ny = 5
    extra_element_integers = 'source_id'
  []
  [stitcher]
    type = StitchedMeshGenerator
    inputs = 'gmg1 gmg2'
    stitch_boundaries_pairs = 'right left'
  []
  [set_material_id0]
    type = SubdomainBoundingBoxGenerator
    input = stitcher
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
    location = INSIDE
    integer_name = material_id
  []
  [set_material_id1]
    type = SubdomainBoundingBoxGenerator
    input = set_material_id0
    bottom_left = '1 0 0'
    top_right = '2 1 0'
    block_id = 2
    location = INSIDE
    integer_name = material_id
  []
  [set_material_id2]
    type = SubdomainBoundingBoxGenerator
    input = set_material_id1
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 3
    location = INSIDE
    integer_name = source_id
  []
  [set_material_id3]
    type = SubdomainBoundingBoxGenerator
    input = set_material_id2
    bottom_left = '1 0 0'
    top_right = '2 1 0'
    block_id = 4
    location = INSIDE
    integer_name = source_id
  []
[]

[AuxVariables]
  [id1]
    family = MONOMIAL
    order = CONSTANT
  []
  [id2]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [id1]
    type = ElementIntegerAux
    variable = id1
    integer_names = material_id
  []
  [id2]
    type = ElementIntegerAux
    variable = id2
    integer_names = source_id
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
