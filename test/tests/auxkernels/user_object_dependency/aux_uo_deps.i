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

[AuxVariables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxKernels]
  [u]
    type = PostprocessorAux
    variable = u
    # this aux kernel is indirectly depending on two other postprocessors, b and c
    pp = a
  []
[]

[Postprocessors]
  [a]
    type = ScalePostprocessor
    value = b
    scaling_factor = 2
  []
  [b]
    type = ScalePostprocessor
    value = c
    scaling_factor = 4
  []
  [c]
    type = VolumePostprocessor
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
