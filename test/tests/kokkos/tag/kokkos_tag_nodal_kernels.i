[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
  [./nodal_ode]
  [../]
[]

[KokkosKernels]
  [./diff]
    type = KokkosDiffusion
    variable = u
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
  [./time]
    type = KokkosTimeDerivative
    variable = u
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
[]

[KokkosNodalKernels]
  [./td]
    type = KokkosTimeDerivativeNodalKernel
    variable = nodal_ode
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
  [./constant_rate]
    type = KokkosConstantRate
    variable = nodal_ode
    rate = 1.0
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1 vec_tag2'
  [../]
[]

[KokkosBCs]
  [./left]
    type = KokkosDirichletBC
    variable = u
    boundary = left
    value = 0
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
  [./right]
    type = KokkosDirichletBC
    variable = u
    boundary = right
    value = 10
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
[]

[Problem]
  type = TagTestProblem
  test_tag_vectors =  'time nontime residual vec_tag1 vec_tag2'
  test_tag_matrices = 'mat_tag1 mat_tag2'

  extra_tag_matrices = 'mat_tag1 mat_tag2'
  extra_tag_vectors  = 'vec_tag1 vec_tag2'
[]

[AuxVariables]
  [./tag_variable1]
    order = FIRST
    family = LAGRANGE
  [../]

  [./tag_variable2]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./TagVectorAux1]
    type = TagVectorAux
    variable = tag_variable1
    v = nodal_ode
    vector_tag = vec_tag2
  [../]

  [./TagVectorAux2]
    type = TagMatrixAux
    variable = tag_variable2
    v = u
    matrix_tag = mat_tag2
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  nl_rel_tol = 1e-08
  dt = 0.01
[]

[Outputs]
  exodus = true
[]
