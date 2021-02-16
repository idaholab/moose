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

[Kernels/diff]
  type = Diffusion
  variable = u
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
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[UserObjects/study]
  type = RepeatableRayStudy
  names = 'line_source_ray'
  start_points = '1 1 0'
  end_points = '5 2 0'
  execute_on = PRE_KERNELS # must be set for line sources!
[]

[RayKernels/line_source]
  type = LineSourceRayKernel
  variable = u
  value = 5
[]

# This isn't used in the test but can be enabled
# for pretty pictures as is used in an example!
[Adaptivity]
  steps = 0 # 5
  marker = marker
  initial_marker = marker
  max_h_level = 5
  [Indicators/indicator]
    type = GradientJumpIndicator
    variable = u
  []
  [Markers/marker]
    type = ErrorFractionMarker
    indicator = indicator
    coarsen = 0.1
    refine = 0.5
  []
[]
