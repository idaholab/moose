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

[Problem]
  kernel_coverage_check = false
[]

[Variables][dummy][][]

[Functions]
  [func]
    type = ConstantFunction
    value = 1
  []
[]

[Postprocessors]
  [c]
    type = FunctionValuePostprocessor
    function = func
    execute_on = initial
    # this will force this postprocessor to be evaluated before transfer on initial
    force_preic = true
  []
[]

[Executioner]
  type = Steady
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = sub.i
    execute_on = initial
  [../]
[]

[Transfers]
  [to_sub]
    type = MultiAppPostprocessorTransfer
    to_multi_app = sub
    from_postprocessor = c
    to_postprocessor = receiver
  []
[]
