[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = MONOMIAL

    [./InitialCondition]
      type = ConstantIC
      value = 1
    [../]
  [../]
[]

[AuxVariables]
  [./tag_variable1]
    order = FIRST
    family = MONOMIAL
  [../]

  [./tag_variable2]
    order = FIRST
    family = MONOMIAL
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

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = 2*pow(e,-x-(y*y))*(1-2*y*y)
  [../]

  [./exact_fn]
    type = ParsedGradFunction
    value = pow(e,-x-(y*y))
    grad_x = -pow(e,-x-(y*y))
    grad_y = -2*y*pow(e,-x-(y*y))
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1 vec_tag2'
  [../]

  [./abs]
    type = Reaction
    variable = u
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1 vec_tag2'
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1'
  [../]
[]

[DGKernels]
  [./dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1 vec_tag2'
  [../]
[]

[BCs]
  [./all]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
    epsilon = -1
    sigma = 6
    extra_matrix_tags = 'mat_tag1 mat_tag2'
    extra_vector_tags = 'vec_tag1 vec_tag2'
  [../]
[]

[Problem]
  type = TagTestProblem
  test_tag_vectors =  'nontime residual vec_tag1 vec_tag2'
  test_tag_matrices = 'mat_tag1 mat_tag2'

  extra_tag_matrices = 'mat_tag1 mat_tag2'
  extra_tag_vectors  = 'vec_tag1 vec_tag2'
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'
  nl_rel_tol = 1e-10
[]

[Postprocessors]
  [./h]
    type = AverageElementSize
  [../]

  [./dofs]
    type = NumDOFs
  [../]

  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Outputs]
  exodus = true
[]
