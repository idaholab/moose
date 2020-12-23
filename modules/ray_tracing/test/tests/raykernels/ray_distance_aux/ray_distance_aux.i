[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmax = 5
  ymax = 5
[]

[AuxVariables/distance]
  order = CONSTANT
  family = MONOMIAL
[]

[RayKernels/distance]
  type = RayDistanceAux
  variable = distance
[]

[UserObjects/study]
  type = LotsOfRaysRayStudy
  vertex_to_vertex = false
  centroid_to_vertex = false
  centroid_to_centroid = true
  execute_on = initial
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
