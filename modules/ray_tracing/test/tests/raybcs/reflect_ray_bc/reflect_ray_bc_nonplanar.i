[Mesh]
  [file]
    type = FileMeshGenerator
    file = nonplanar.e
  []
  [subdomains]
    type = ParsedSubdomainMeshGenerator
    input = file
    combinatorial_geometry = 'x > 0.5'
    block_id = 1
  []
  [internal_sideset]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomains
    primary_block = 0
    paired_block = 1
    new_boundary = internal
  []
[]

[UserObjects/study]
  type = RepeatableRayStudy
  start_points = '0 0 0'
  directions = '1 1 1'
  names = 'ray'
  warn_non_planar = false
  use_internal_sidesets = true
  execute_on = initial
  tolerate_failure = true
[]

[RayBCs]
  [kill]
    type = KillRayBC
    boundary = 'top right bottom left front back'
  []
  [reflect_internal]
    type = ReflectRayBC
    boundary = internal
  []
[]

[RayKernels/null]
  type = NullRayKernel
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
