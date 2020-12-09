[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 5
    xmax = 3
    ymax = 5
  []
  [subdomain1]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'x > 1 & x < 2'
    block_id = 1
  []
  [subdomain2]
    type = ParsedSubdomainMeshGenerator
    input = subdomain1
    combinatorial_geometry = 'x > 2'
    block_id = 2
  []
[]

[AuxVariables/aux]
  order = CONSTANT
  family = MONOMIAL
[]

[RayKernels]
  [aux0] # add the value of 1 to aux for all Rays that pass through block 0
    type = FunctionAuxRayKernelTest
    variable = aux
    function = 1
    block = 0
  []
  [aux1] # add the value of 2 to aux for all Rays that pass through block 1
    type = FunctionAuxRayKernelTest
    variable = aux
    function = 2
    block = 1
  []
  [aux2] # add the value of 3 to aux for all Rays that pass through block 2
    type = FunctionAuxRayKernelTest
    variable = aux
    function = 3
    block = 2
  []
[]

[UserObjects/lots]
  type = LotsOfRaysRayStudy
  execute_on = initial

  vertex_to_vertex = false
  centroid_to_vertex = false
  centroid_to_centroid = true
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
