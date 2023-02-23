[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 8
    ymin = 0
    ymax = 8
    nx = 8
    ny = 8
  []
  [coarse_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 8
    ymin = 0
    ymax = 8
    nx = 3
    ny = 3
  []
  [coarse_id]
    type = CoarseMeshExtraElementIDGenerator
    input = gmg
    coarse_mesh = coarse_mesh
    extra_element_id_name = coarse_elem_id
    enforce_mesh_embedding = false
  []
  # need this to ensure consistent numbering of the coarse mesh when using a distributed mesh
  allow_renumbering = false
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[AuxVariables]
  [coarse_elem_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [coarse_elem_id]
    type = ExtraElementIDAux
    variable = coarse_elem_id
    extra_id_name = coarse_elem_id
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
