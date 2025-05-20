[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Variables][dummy][][]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Steady
[]

[Adaptivity]
  marker = uniform
  steps = 1

  [Markers/uniform]
    type = UniformMarker
    mark = refine
  []
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = sub.i
    execute_on = post_adaptivity
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldShapeEvaluationTransfer
    to_multi_app = sub
    source_variable = uniform
    variable = uniform
  []
[]
