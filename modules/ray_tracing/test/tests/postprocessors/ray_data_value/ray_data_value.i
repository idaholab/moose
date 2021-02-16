[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 5
    ny = 5
    xmax = 5
    ymax = 5
  []
[]

[RayKernels/null]
  type = NullRayKernel
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0.5 0.5 0
                  2.5 2.5 0'
  end_points = '5 4.9 0
                0.1 0 0'
  names = 'ray0 ray1'
  ray_data_names = 'data0 data1'
  initial_ray_data = '1 2;
                      3 4'
  ray_aux_data_names = 'aux_data0 aux_data1'
  initial_ray_aux_data = '5 6;
                          7 8'
[]

[Postprocessors]
  [ray0_data0]
    type = RayDataValue
    study = study
    ray_name = ray0
    data_name = data0
  []
  [ray0_data1]
    type = RayDataValue
    study = study
    ray_name = ray0
    data_name = data1
  []
  [ray0_aux_data0]
    type = RayDataValue
    study = study
    ray_name = ray0
    data_name = aux_data0
    aux = true
  []
  [ray0_aux_data1]
    type = RayDataValue
    study = study
    ray_name = ray0
    data_name = aux_data1
    aux = true
  []

  # For ray1, we're betting on the fact that the IDs are assigned
  # in sequential order, therefore its ID should be 1. In reality,
  # you should rely on the name but this is just for testing purposes.
  [ray1_data0]
    type = RayDataValue
    study = study
    ray_id = 1
    data_name = data0
  []
  [ray1_data1]
    type = RayDataValue
    study = study
    ray_id = 1
    data_name = data1
  []
  [ray1_aux_data0]
    type = RayDataValue
    study = study
    ray_id = 1
    data_name = aux_data0
    aux = true
  []
  [ray1_aux_data1]
    type = RayDataValue
    study = study
    ray_id = 1
    data_name = aux_data1
    aux = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = false
  csv = true
[]
