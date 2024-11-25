[Problem]
  extra_tag_vectors = zeroed_tag
  not_zeroed_tag_vectors = not_zeroed_tag
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  xmin = -1
  xmax = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [null]
    type = NullKernel
    variable = u
  []
[]

[Functions]
  [switch_off]
    type = ParsedFunction
    expression = 'if(t < 1.0001, 1, 0)'
  []
[]

[DiracKernels]
  [point_source1]
    type = FunctionDiracSource
    variable = u
    function = switch_off
    point = '0 0 0'
    vector_tags = 'zeroed_tag not_zeroed_tag'
  []
[]

[AuxVariables]
  [not_zeroed_tag]
  []
  [zeroed_tag]
  []
[]

[AuxKernels]
  [not_zeroed_tag_value]
    type = TagVectorAux
    variable = not_zeroed_tag
    vector_tag = not_zeroed_tag
    v = u
  []
  [zeroed_tag_value]
    type = TagVectorAux
    variable = zeroed_tag
    vector_tag = zeroed_tag
    v = u
  []
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 2
[]

[Outputs]
  exodus = true
[]
