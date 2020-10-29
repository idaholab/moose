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
  names = ray
  ray_data_names = data
  initial_ray_data = 1
[]

[RayBCs/kill]
  type = KillRayBC
  boundary = right
  rays = ray
[]

[RayKernels]
  [add_1]
    type = ChangeDataRayKernelTest
    data_name = data
    add_value = 1
    rays = ray
    depends_on = add_10
  []
  [scale_5]
    type = ChangeDataRayKernelTest
    data_name = data
    scale_value = 5
    rays = ray
    depends_on = scale_9
  []
  [add_10]
    type = ChangeDataRayKernelTest
    data_name = data
    add_value = 10
    rays = ray
  []
  [scale_9]
    type = ChangeDataRayKernelTest
    data_name = data
    scale_value = 9
    rays = ray
    depends_on = add_1
  []
[]

[Postprocessors/value]
  type = RayDataValue
  study = study
  ray_name = ray
  data_name = data
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
