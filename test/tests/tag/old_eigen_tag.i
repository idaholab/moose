[Mesh/gmg]
  type = GeneratedMeshGenerator
  dim = 2
  nx = 5
  ny = 5
[]

[Variables]
  [u]
  []
  [v]
  []
[]

[AuxVariables]
  [u_vec_tag_diff]
  []
  [u_vec_tag_rhs]
  []
  [u_mat_tag_diff]
  []
  [u_mat_tag_rhs]
  []
  [u_mat_savein_rhs]
  []

  [v_vec_tag_diff]
  []
  [v_vec_tag_rhs]
  []
  [v_mat_tag_diff]
  []
  [v_mat_tag_rhs]
  []
[]


[Kernels]
  [diff_u]
    type = Diffusion
    variable = u
    extra_vector_tags = 'tag_diff'
    extra_matrix_tags = 'tag_diff'
  []
  [rhs_u]
    type = MassEigenKernel
    variable = u
    extra_vector_tags = 'tag_rhs'
    extra_matrix_tags = 'tag_rhs'
    diag_save_in = 'u_mat_savein_rhs'
  []

  [diff_v]
    type = Diffusion
    variable = v
    extra_vector_tags = 'tag_diff'
    extra_matrix_tags = 'tag_diff'
  []
  [rhs_v]
    type = MassEigenKernel
    variable = v
    extra_vector_tags = 'tag_rhs'
    extra_matrix_tags = 'tag_rhs'
  []

  [rhs_uv]
    type = CoupledEigenKernel
    variable = u
    v = v
    extra_vector_tags = 'tag_rhs'
    extra_matrix_tags = 'tag_rhs'
  []
  [rhs_vu]
    type = CoupledEigenKernel
    variable = v
    v = u
    extra_vector_tags = 'tag_rhs'
    extra_matrix_tags = 'tag_rhs'
  []
[]

[AuxKernels]
  [u_vec_tag_diff]
    type = TagVectorAux
    variable = u_vec_tag_diff
    v = u
    vector_tag = tag_diff
  []
  [u_vec_tag_rhs]
    type = TagVectorAux
    variable = u_vec_tag_rhs
    v = u
    vector_tag = tag_rhs
  []
  [u_mat_tag_diff]
    type = TagVectorAux
    variable = u_mat_tag_diff
    v = u
    vector_tag = tag_diff
  []
  [u_mat_tag_rhs]
    type = TagVectorAux
    variable = u_mat_tag_diff
    v = u
    vector_tag = tag_rhs
  []

  [v_vec_tag_diff]
    type = TagVectorAux
    variable = v_vec_tag_diff
    v = v
    vector_tag = tag_diff
  []
  [v_vec_tag_rhs]
    type = TagVectorAux
    variable = v_vec_tag_rhs
    v = v
    vector_tag = tag_rhs
  []
  [v_mat_tag_diff]
    type = TagVectorAux
    variable = v_mat_tag_diff
    v = v
    vector_tag = tag_diff
  []
  [v_mat_tag_rhs]
    type = TagVectorAux
    variable = v_mat_tag_diff
    v = v
    vector_tag = tag_rhs
  []
[]

[BCs]
  [homogeneous_u]
    type = DirichletBC
    boundary = 'top right bottom left'
    variable = u
    value = 0
  []
  [homogeneous_v]
    type = DirichletBC
    boundary = 'top right bottom left'
    variable = v
    value = 0
  []
[]

[Problem]
  extra_tag_vectors = 'tag_diff tag_rhs'
  extra_tag_matrices = 'tag_diff tag_rhs'
[]

[Postprocessors]
  [unorm]
    type = NodalL2Norm
    variable = u
    execute_on = linear
  []
  [vnorm]
    type = NodalL2Norm
    variable = v
    execute_on = linear
  []
  [uvnorm]
    type = ParsedPostprocessor
    function = 'sqrt(unorm*unorm + vnorm*vnorm)'
    pp_names = 'unorm vnorm'
    execute_on = linear
  []
[]

[Preconditioning/smp]
  type = SMP
  full = true
[]

[Executioner]
  type = NonlinearEigen
  bx_norm = uvnorm
  free_l_tol = 1e-8
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
