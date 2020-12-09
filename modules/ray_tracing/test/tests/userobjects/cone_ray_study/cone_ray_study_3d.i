[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 5
    ny = 5
    nz = 2
    xmax = 5
    ymax = 5
    zmax = 2
  []
[]

[Variables/u]
[]

[Kernels]
  [reaction]
    type = Reaction
    variable = u
  []
  [diffusion]
    type = Diffusion
    variable = u
  []
[]

[UserObjects/study]
  type = ConeRayStudy
  start_points = '2.5 2.5 0'
  directions = '0 0 1'
  half_cone_angles = 10

  # Must be set with RayKernels that
  # contribute to the residual
  execute_on = PRE_KERNELS

  # For outputting Rays
  always_cache_traces = true

  ray_data_name = weight
[]

[RayKernels/null]
  type = NullRayKernel
[]

# Rays only hit the front surface
[RayBCs/kill]
  type = KillRayBC
  boundary = 'front'
[]

[RayKernels/line_source]
  type = LineSourceRayKernel
  variable = u

  # Scale by the weights in the ConeRayStudy
  ray_data_factor_names = weight
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true

  [rays]
    type = RayTracingExodus
    study = study
    execute_on = FINAL
  []
[]

[Adaptivity]
  steps = 0 # 6 for pretty pictures
  marker = marker
  initial_marker = marker
  max_h_level = 6
  [Indicators/indicator]
    type = GradientJumpIndicator
    variable = u
  []
  [Markers/marker]
    type = ErrorFractionMarker
    indicator = indicator
    coarsen = 0.25
    refine = 0.5
  []
[]
