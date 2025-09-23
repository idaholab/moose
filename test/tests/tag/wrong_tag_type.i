[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 10
  []
[]

[Problem]
  solve = false
  extra_tag_vectors = 'vec_tag1'
  extra_tag_solutions = 'vec_tag2'
[]

[Variables]
  [u]
    initial_condition = 1
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    extra_vector_tags = 'vec_tag1'
  []
  [react]
    type = Reaction
    variable = u
    extra_vector_tags = 'vec_tag1 vec_tag2'
  []
[]

[AuxVariables]
  [tag_value]
    [AuxKernel]
      type = TagVectorAux
      v = u
      vector_tag = vec_tag2
      remove_variable_scaling = true
    []
  []
[]

[Executioner]
  type = Steady
[]
