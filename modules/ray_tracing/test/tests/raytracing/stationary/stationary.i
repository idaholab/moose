[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 2
  nx = 5
  ny = 6
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[UserObjects/study]
  type = StationaryRayStudyTest
[]

[AuxVariables/aux]
  order = CONSTANT
  family = MONOMIAL
[]

[RayKernels/aux]
  type = FunctionAuxRayKernelTest
  variable = aux
  function = 'x + 2 * y'
[]

[Outputs]
  exodus = true
[]
