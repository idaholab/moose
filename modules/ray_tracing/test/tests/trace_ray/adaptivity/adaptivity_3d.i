[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
[]

[Variables/u]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  steps = 1
  marker = marker
  initial_marker = marker
  max_h_level = 2
  [Indicators/indicator]
    type = GradientJumpIndicator
    variable = u
  []
  [Markers/marker]
    type = ErrorFractionMarker
    indicator = indicator
    coarsen = 0.1
    refine = 0.1
  []
[]

[UserObjects/study]
  type = LotsOfRaysRayStudy
  ray_kernel_coverage_check = false
  vertex_to_vertex = true
  centroid_to_vertex = true
  centroid_to_centroid = true
  execute_on = timestep_end
[]

[RayBCs/kill]
  type = KillRayBC
  boundary = 'top right bottom left front back'
[]

[Postprocessors]
  [total_distance]
    type = RayTracingStudyResult
    study = study
    result = total_distance
    execute_on = timestep_end
  []
  [total_rays]
    type = RayTracingStudyResult
    study = study
    result = total_rays_started
    execute_on = timestep_end
  []
[]

[Outputs]
  exodus = false
  csv = true
[]
