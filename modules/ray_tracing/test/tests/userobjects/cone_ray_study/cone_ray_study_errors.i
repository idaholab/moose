[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
  []
[]

[UserObjects/study]
  type = ConeRayStudy
  start_points = '1 1.5 0'
  directions = '2 1 0'
  half_cone_angles = 2.5
  ray_data_name = weight
[]

[Executioner]
  type = Steady
[]
