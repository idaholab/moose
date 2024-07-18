[Problem]
  extra_tag_vectors = zeroed_tag
  not_zeroed_tag_vectors = manual_tag
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    extra_vector_tags = 'manual_tag zeroed_tag'
  []
[]


[AuxVariables]
  [manual_tag]
  []
  [zeroed_tag]
  []
[]

[AuxKernels]
  [manual_tag_value]
    type = TagVectorAux
    variable = manual_tag
    vector_tag = manual_tag
    v = u
  []
  [zeroed_tag_value]
    type = TagVectorAux
    variable = zeroed_tag
    vector_tag = zeroed_tag
    v = u
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
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
