[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/study]
  type = RayTracingStudyTest
[]

[RayKernels/test]
  type = RayTracingObjectTest
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
