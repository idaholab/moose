[Mesh]
  [square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  []
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[AuxVariables]
  [tag_variable1]
    order = FIRST
    family = LAGRANGE
  []

  [tag_variable2]
    order = FIRST
    family = LAGRANGE
  []
[]

[KokkosKernels]
  [diff]
    type = KokkosDiffusion
    variable = u
    extra_vector_tags = 'vec_tag1 vec_tag2'
  []
[]

[KokkosAuxKernels]
  [TagVectorAux1]
    type = KokkosTagVectorAux
    variable = tag_variable1
    v = u
    vector_tag = vec_tag1
  []
  [TagVectorAux2]
    type = KokkosTagVectorAux
    variable = tag_variable2
    v = u
    vector_tag = vec_tag2
  []
[]

[KokkosBCs]
  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = 3
    value = 0
    extra_vector_tags = vec_tag1
  []

  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = 1
    value = 1
    extra_vector_tags = vec_tag2
  []
[]

[Problem]
  type = FEProblem
  extra_tag_vectors  = 'vec_tag1 vec_tag2'
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-vec_type -nl0_mat_type'
  petsc_options_value = 'kokkos hypre'
[]

[Outputs]
  exodus = true
[]
