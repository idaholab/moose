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

  [end_study]
    type = RepeatableRayStudy
    start_points = '0 0 0'
    end_points = '0.25 0 0'
    names = 'ray'
    execute_on = INITIAL
    ray_kernel_coverage_check = false
  []
[]

[RayKernels]
  active = ''

  [kill]
    type = KillRayKernel
  []
  [change_after_kill]
    type = ChangeRayRayKernelTest
    change_start_direction = true
    depends_on = kill
  []

  [change]
    type = ChangeRayRayKernelTest
    change_start_direction = true
  []
  [change_again]
    type = ChangeRayRayKernelTest
    change_start_direction = true
  []

  [change_out_of_elem]
    type = ChangeRayRayKernelTest
    change_start_out_of_elem = true
  []

  [change_zero]
    type = ChangeRayRayKernelTest
    change_direction_zero = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
