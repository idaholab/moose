[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./tag_variable]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]

  [./diff]
    type = Diffusion
    variable = u
    extra_matrix_tags = 'mat_tag1 mat_tag2'
  [../]

  [./diff1]
    type = Diffusion
    variable = u
    extra_matrix_tags = 'mat_tag2'
    vector_tags = vec_tag1
  [../]

  [./diff2]
    type = Diffusion
    variable = u
    vector_tags = vec_tag1
  [../]

  [./diff3]
    type = Diffusion
    variable = u
    vector_tags = vec_tag1
  [../]
[]

[AuxKernels]
  [./TagMatrixAux]
    type = TagMatrixAux
    variable = tag_variable
    v = u
    matrix_tag = mat_tag2
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
    extra_matrix_tags = mat_tag1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
    extra_matrix_tags = mat_tag1
  [../]
[]

[Problem]
  type = TagTestProblem
  test_tag_vectors =  'nontime residual'
  test_tag_matrices = 'mat_tag1 mat_tag2'
  extra_tag_matrices = 'mat_tag1 mat_tag2'
  extra_tag_vectors  = 'vec_tag1'
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
[]

[Outputs]
  file_base = matrix_tag_test_out
  exodus = true
[]
