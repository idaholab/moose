[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
  uniform_refine = 4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./ddt_u]
    type = TimeDerivative
    variable = u
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]

  [./diff_u]
    type = Diffusion
    variable = u
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]

  [./ddt_v]
    type = TimeDerivative
    variable = v
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
[]

[DiracKernels]
  [./nonlinear_source]
    type = NonlinearSource
    variable = u
    coupled_var = v
    scale_factor = 1000
    point = '0.2 0.3 0'
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1 vec_tag2'
  [../]
[]

[BCs]
  [./left_u]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]

  [./right_u]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = 3
    value = 1
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = 1
    value = 0
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
[]

[Preconditioning]
  [./precond]
    type = SMP
    full = true
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
    v = u
    vector_tag = vec_tag2
    execute_on = timestep_end
  [../]

  [./TagVectorAux2]
    type = TagMatrixAux
    variable = tag_variable2
    v = u
    matrix_tag = mat_tag2
    execute_on = timestep_end
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON' # NEWTON provides a more stringent test of off-diagonal Jacobians
  num_steps = 5
  dt = 1
  dtmin = 1
  l_max_its = 100
  nl_max_its = 6
  nl_abs_tol = 1.e-13
[]

[Postprocessors]
  [./point_value]
    type = PointValue
    variable = u
    point = '0.2 0.3 0'
  [../]
[]

[Outputs]
  exodus = true
[]
