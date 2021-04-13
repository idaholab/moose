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

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    extra_matrix_tags = 'mat_tag1 mat_tag2'
  []
[]

[AuxKernels]
  [TagMatrixAux1]
    type = TagMatrixAux
    variable = tag_variable1
    v = u
    matrix_tag = mat_tag1
  []

  [TagMatrixAux2]
    type = TagMatrixAux
    variable = tag_variable2
    v = u
    matrix_tag = mat_tag2
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
    extra_matrix_tags = mat_tag1
  []

  [right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
    extra_matrix_tags = mat_tag2
  []
[]

[Problem]
  type = FEProblem
  extra_tag_matrices = 'mat_tag1 mat_tag2'
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
[]

[Outputs]
  file_base = tag_matrix_out
  exodus = true
[]
