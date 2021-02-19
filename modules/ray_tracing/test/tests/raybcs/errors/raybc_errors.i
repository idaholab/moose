[Mesh]
  active = 'gmg'

  [gmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = '0.5 0.5'
    ix = '1 1'
    subdomain_id = '0 1'
  []

  [internal]
    type = SideSetsBetweenSubdomainsGenerator
    input = gmg
    primary_block = 1
    paired_block = 0
    new_boundary = internal
  []
[]

[UserObjects]
  active = 'study'
  [study]
    type = RepeatableRayStudy
    start_points = '0 0 0'
    directions = '1 0 0'
    names = 'ray'
    execute_on = INITIAL
    ray_kernel_coverage_check = false
  []

  [set_end_study]
    type = RepeatableRayStudy
    start_points = '0 0 0'
    end_points = '1 0 0'
    names = 'ray'
    execute_on = INITIAL
    ray_kernel_coverage_check = false
    use_internal_sidesets = true
  []

  [start_internal_study]
    type = RepeatableRayStudy
    start_points = '0.5 0 0'
    directions = '1 0 0'
    names = 'ray'
    execute_on = INITIAL
    ray_kernel_coverage_check = false
    use_internal_sidesets = true
  []
[]

[RayBCs]
  active = ''

  [kill]
    type = KillRayBC
    boundary = right
  []
  [change]
    type = ChangeRayRayBCTest
    boundary = right
    change_direction = true
  []
  [change_again]
    type = ChangeRayRayBCTest
    boundary = right
    change_direction = true
  []
  [change_internal]
    type = ChangeRayRayBCTest
    boundary = internal
    change_direction = true
  []
  [change_zero]
    type = ChangeRayRayBCTest
    boundary = right
    change_direction_zero = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
