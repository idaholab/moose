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

[Postprocessors]
  [scaled]
    type = ScalePostprocessor
    value = receiver
    scaling_factor = 2
    # Note: during subapp initial setup, parent postprocessor has not been transferred
    execute_on = 'initial timestep_end'
  []
  [receiver]
    type = Receiver
    default = 0
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
