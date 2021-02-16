[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  active = ''
  [study]
    type = RepeatableRayStudy
    start_points = '0 0 0'
    directions = '1 0 0'
    names = 'ray'
  []
  [another_study]
    type = RepeatableRayStudy
    start_points = '0 0 0'
    directions = '1 0 0'
    names = 'ray'
  []
  [not_a_study]
    type = VerifyElementUniqueID
  []
[]

[RayKernels]
  active = ''
  [missing_study_by_name]
    type = NullRayKernel
    study = dummy
  []
  [not_a_study]
    type = NullRayKernel
    study = not_a_study
  []
  [multiple_studies]
    type = NullRayKernel
  []
  [missing_study]
    type = NullRayKernel
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
