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

# to_right_distance_kill - makes it to the right boundary at (5, 0)
#   and dies due to max distance (doesn't call RayBCs)
# to_right_bc_kill - makes it to right boundary at (5, 0); is still
#   0.1 from its max distance so calls 'kill_right' RayBC which
#   kills it
# to_top_corner - makes it to the top right corner at (5, 5);
#   reflects with direction (-1, -1) and stops once its distance
#   hits 7.0
# reflect_a_lot - reflects a bunch with the RayBC 'reflect_all'
#   until it gets to a distance of 50 and dies
[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0
                  0 0 0
                  0 0 0
                  0.1 0.2 0'
  directions = '1 0 0
                1 0 0
                1 1 0
                1 0.5 0'
  max_distances = '5
                   5.1
                   7.0
                   50'
  names = 'to_right_distance_kill
           to_right_bc_kill
           to_top_corner
           reflect_a_lot'
[]

[RayKernels/null]
  type = NullRayKernel
[]

[RayBCs]
  [kill_right]
    type = KillRayBC
    boundary = right
    rays = 'to_right_bc_kill'
  []
  [reflect_top_right]
    type = ReflectRayBC
    boundary = 'top right'
    rays = 'to_top_corner'
  []
  [reflect_all]
    type = ReflectRayBC
    boundary = 'top right bottom left'
    rays = 'reflect_a_lot'
  []
[]

[Postprocessors/total_distance]
  type = RayTracingStudyResult
  result = 'total_distance'
  study = study
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
