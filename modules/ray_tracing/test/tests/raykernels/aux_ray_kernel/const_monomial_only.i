[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0'
  directions = '1 0 0'
  names = 'ray'
  execute_on = INITIAL
[]

[AuxVariables/aux]
[]

[RayKernels/distance]
  type = RayDistanceAux
  variable = aux
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
