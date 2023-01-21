[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 1
  ny = 1
  elem_type = QUAD4
[]

[Variables]
  [./n]
    family = SCALAR
    order = FIRST
    initial_condition = 1
  [../]
[]

[AuxVariables]
  [./tag_vector_var1]
    family = SCALAR
    order = FIRST
  [../]
  [./tag_vector_var2]
    family = SCALAR
    order = FIRST
  [../]
  [./tag_matrix_var2]
    family = SCALAR
    order = FIRST
  [../]
[]

[ScalarKernels]
  [./dn]
    type = ODETimeDerivative
    variable = n
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]

  [./ode1]
    type = ParsedODEKernel
    expression = '-n'
    variable = n
    extra_matrix_tags = 'mat_tag1'
    extra_vector_tags = 'vec_tag1'
  [../]

  [./ode2]
    type = ParsedODEKernel
    expression = '-n'
    variable = n
    vector_tags = 'vec_tag2'
    matrix_tags = 'mat_tag2'
  [../]
[]

[AuxScalarKernels]
  [./TagVectorAux]
    type = ScalarTagVectorAux
    variable = tag_vector_var1
    v = n
    vector_tag  = vec_tag1
    execute_on = timestep_end
  [../]

  [./TagVectorAux2]
    type = ScalarTagVectorAux
    variable = tag_vector_var2
    v = n
    vector_tag  = vec_tag2
    execute_on = timestep_end
  [../]

  [./TagMatrixAux2]
    type = ScalarTagMatrixAux
    variable = tag_matrix_var2
    v = n
    matrix_tag  = mat_tag2
    execute_on = timestep_end
  [../]
[]

[Problem]
  type = TagTestProblem
  test_tag_vectors =  'time nontime residual vec_tag1 vec_tag2'
  test_tag_matrices = 'mat_tag1 mat_tag2'

  extra_tag_matrices = 'mat_tag1 mat_tag2'
  extra_tag_vectors  = 'vec_tag1 vec_tag2'
[]

[Executioner]
  type = Transient
  start_time = 0
  num_steps = 10
  dt = 0.001
  dtmin = 0.001 # Don't allow timestep cutting
  solve_type = NEWTON
  nl_max_its = 2
  nl_abs_tol = 1.e-12 # This is an ODE, so nl_abs_tol makes sense.
[]

[Functions]
  [./exact_solution]
    type = ParsedFunction
    expression = exp(t)
  [../]
[]

[Postprocessors]
  [./error_n]
    # Post processor that computes the difference between the computed
    # and exact solutions.  For the exact solution used here, the
    # error at the final time should converge at O(dt^p), where p is
    # the order of the method.
    type = ScalarL2Error
    variable = n
    function = exact_solution
    # final is not currently supported for Postprocessor execute_on...
    # execute_on = 'final'
  [../]
[]

[Outputs]
  csv = true
[]
