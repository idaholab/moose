[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 2
  nx = 5
  ny = 5
[]

[Variables/u]
[]

[AuxVariables]
  [vec_tag_diff]
    order = FIRST
    family = LAGRANGE
  []
  [vec_tag_rhs]
    order = FIRST
    family = LAGRANGE
  []
  [vec_tag_combined]
    order = FIRST
    family = LAGRANGE
  []
  [mat_tag_diff]
    order = FIRST
    family = LAGRANGE
  []
  [mat_tag_rhs]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    extra_vector_tags = 'tag_diff tag_combined'
    extra_matrix_tags = 'tag_diff'
  []
  [rhs]
    type = CoefReaction
    variable = u
    coefficient = -1
    extra_vector_tags = 'eigen tag_rhs tag_combined'
    extra_matrix_tags = 'tag_rhs'
  []
[]

[AuxKernels]
  [vec_tag_diff]
    type = TagVectorAux
    variable = vec_tag_diff
    v = u
    vector_tag = tag_diff
  []
  [vec_tag_rhs]
    type = TagVectorAux
    variable = vec_tag_rhs
    v = u
    vector_tag = tag_rhs
  []
  [vec_tag_combined]
    type = TagVectorAux
    variable = vec_tag_combined
    v = u
    vector_tag = tag_combined
  []

  [mat_tag_diff]
    type = TagVectorAux
    variable = mat_tag_diff
    v = u
    vector_tag = tag_diff
  []
  [mat_tag_rhs]
    type = TagVectorAux
    variable = mat_tag_diff
    v = u
    vector_tag = tag_rhs
  []
[]

[BCs]
  [homogeneous]
    type = DirichletBC
    boundary = 'top right bottom left'
    variable = u
    value = 0
    extra_vector_tags = 'tag_combined'
  []
  [eigen]
    type = EigenDirichletBC
    variable = u
    boundary = 'top right bottom left'
  []
[]

[Problem]
  type = EigenProblem
  extra_tag_vectors = 'tag_diff tag_rhs tag_combined'
  extra_tag_matrices = 'tag_diff tag_rhs'
  bx_norm = uint
[]

[Postprocessors]
  [combined_res_norm]
    # because all kernels, BCs are tagged, this postprocessor is expected to be zero within tolerance
    type = ElementL2Norm
    variable = vec_tag_combined
  []
  [uint]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'initial linear'
  []
[]

[Executioner]
  type = Eigenvalue
  solve_type = NEWTON
  eigen_problem_type = GEN_NON_HERMITIAN
[]

[Outputs]
  exodus = true
[]
