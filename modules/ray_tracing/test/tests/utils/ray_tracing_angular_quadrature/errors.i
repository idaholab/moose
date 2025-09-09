[Application]
  allow_test_objects = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects/test]
  type = RayTracingAngularQuadratureErrorTest
[]

[Executioner]
  type = Steady
[]
