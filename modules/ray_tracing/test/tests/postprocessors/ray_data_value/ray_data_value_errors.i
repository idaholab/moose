[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  active = ''

  [lots]
    type = LotsOfRaysRayStudy
    vertex_to_vertex = false
    centroid_to_centroid = false
    centroid_to_vertex = false
    ray_kernel_coverage_check = false
  []
  [repeatable]
    type = RepeatableRayStudy
    start_points = '0 0 0'
    directions = '1 0 0'
    ray_data_names = data
    names = ray
    ray_kernel_coverage_check = false
  []
  [no_banking_study]
    type = DisableRayBankingStudy
    start_points = '0 0 0'
    directions = '1 0 0'
    ray_data_names = data
    names = ray
    ray_kernel_coverage_check = false
  []
[]

[RayBCs]
  active = ''
  [kill]
    type = KillRayBC
    boundary = right
  []
[]

[Postprocessors]
  active = ''

  [no_registration]
    type = RayDataValue
    study = lots
    ray_name = dummy
    data_name = dummy
  []
  [no_banking]
    type = RayDataValue
    study = no_banking_study
    ray_name = ray
    data_name = data
  []
  [ray_name_not_found]
    type = RayDataValue
    study = repeatable
    ray_name = dummy
    data_name = data
    execute_on = initial
  []
  [data_name_not_found]
    type = RayDataValue
    study = repeatable
    ray_name = ray
    data_name = dummy
    execute_on = initial
  []
  [aux_data_name_not_found]
    type = RayDataValue
    study = repeatable
    ray_name = ray
    data_name = dummy
    aux = true
    execute_on = initial
  []
  [id_not_found]
    type = RayDataValue
    study = repeatable
    ray_id = 1
    data_name = data
    execute_on = final
  []
  [neither_provided]
    type = RayDataValue
    study = repeatable
    data_name = data
    execute_on = initial
  []
  [both_provided]
    type = RayDataValue
    study = repeatable
    data_name = data
    ray_id = 0
    ray_name = dummy
    execute_on = initial
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
