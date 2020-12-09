[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 5
    ymax = 5
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

[Executioner]
  type = Steady
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[UserObjects/study]
  type = ConeRayStudy

  start_points = '1 1.5 0'
  directions = '2 1 0'
  half_cone_angles = 2.5
  ray_data_name = weight

  # Must be set with RayKernels that
  # contribute to the residual
  execute_on = PRE_KERNELS

  # For outputting Rays
  always_cache_traces = true
[]

[RayBCs]
  [reflect]
    type = ReflectRayBC
    boundary = 'right'
  []
  [kill_rest]
    type = KillRayBC
    boundary = 'top'
  []
[]

[RayKernels/line_source]
  type = LineSourceRayKernel
  variable = u

  # Scale by the weights in the ConeRayStudy
  ray_data_factor_names = weight
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
