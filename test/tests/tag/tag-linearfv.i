[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 3
  []
[]

[Problem]
  linear_sys_names = 'u_sys v_sys'
  extra_tag_matrices = 'mat_tag_u; mat_tag_v'
  extra_tag_vectors = 'vec_tag_u; vec_tag_v'
[]

[Variables]
  [u]
    type = MooseLinearVariableFVReal
    initial_condition = 1.0
    solver_sys = u_sys
  []
  [v]
    type = MooseLinearVariableFVReal
    initial_condition = 0.5
    solver_sys = v_sys
  []
[]

[LinearFVKernels]
  [diffusion]
    type = LinearFVDiffusion
    variable = u
    diffusion_coeff = 2.0
  []
  [reaction]
    type = LinearFVReaction
    variable = u
    coeff = 3.0
    matrix_tags = 'system mat_tag_u'
  []
  [source]
    type = LinearFVSource
    variable = u
    source_density = 60.0
    vector_tags = 'rhs vec_tag_u'
  []
  [diffusion_v]
    type = LinearFVDiffusion
    variable = v
    diffusion_coeff = 1.0
  []
  [reaction_v]
    type = LinearFVReaction
    variable = v
    coeff = 1.5
    matrix_tags = 'system mat_tag_v'
  []
  [source_v]
    type = LinearFVSource
    variable = v
    source_density = 20.0
    vector_tags = 'rhs vec_tag_v'
  []
[]

[LinearFVBCs]
  [left_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = left
    functor = 1.0
  []
  [right_u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = u
    boundary = right
    functor = 3.0
  []
  [left_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = v
    boundary = left
    functor = 1.0
  []
  [right_v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    variable = v
    boundary = right
    functor = 3.0
  []
[]

[AuxVariables]
  [soln_u_dof]
    type = MooseLinearVariableFVReal
  []
  [soln_u]
    type = MooseLinearVariableFVReal
  []
  [rhs_u_dof]
    type = MooseLinearVariableFVReal
  []
  [rhs_u]
    type = MooseLinearVariableFVReal
  []
  [vector_tag_u]
    type = MooseLinearVariableFVReal
  []
  [matrix_u_diag]
    type = MooseLinearVariableFVReal
  []
  [soln_v_dof]
    type = MooseLinearVariableFVReal
  []
  [soln_v]
    type = MooseLinearVariableFVReal
  []
  [rhs_v_dof]
    type = MooseLinearVariableFVReal
  []
  [rhs_v]
    type = MooseLinearVariableFVReal
  []
  [vector_tag_v]
    type = MooseLinearVariableFVReal
  []
  [matrix_v_diag]
    type = MooseLinearVariableFVReal
  []
[]

[AuxKernels]
  [soln_u_dof]
    type = TagVectorDofValueAux
    variable = soln_u_dof
    v = u
    vector_tag = 'solution'
  []
  [soln_u]
    type = TagVectorAux
    variable = soln_u
    v = u
    vector_tag = 'solution'
  []
  [rhs_u_dof]
    type = TagVectorDofValueAux
    variable = rhs_u_dof
    v = u
    vector_tag = 'rhs'
  []
  [rhs_u]
    type = TagVectorAux
    variable = rhs_u
    v = u
    vector_tag = 'rhs'
  []
  [extra_vector_u_dof]
    type = TagVectorDofValueAux
    variable = vector_tag_u
    v = u
    vector_tag = 'vec_tag_u'
  []
  [extra_vector_u]
    type = TagVectorAux
    variable = vector_tag_u
    v = u
    vector_tag = 'vec_tag_u'
  []
  [extra_matrix_u]
    type = TagMatrixAux
    variable = matrix_u_diag
    v = u
    matrix_tag = 'mat_tag_u'
  []
  [soln_v_dof]
    type = TagVectorDofValueAux
    variable = soln_v_dof
    v = v
    vector_tag = 'solution'
  []
  [soln_v]
    type = TagVectorAux
    variable = soln_v
    v = v
    vector_tag = 'solution'
  []
  [rhs_v_dof]
    type = TagVectorDofValueAux
    variable = rhs_v_dof
    v = v
    vector_tag = 'rhs'
  []
  [rhs_v]
    type = TagVectorAux
    variable = rhs_v
    v = v
    vector_tag = 'rhs'
  []
  [extra_vector_v_dof]
    type = TagVectorDofValueAux
    variable = vector_tag_v
    v = v
    vector_tag = 'vec_tag_v'
  []
  [extra_vector_v]
    type = TagVectorAux
    variable = vector_tag_v
    v = v
    vector_tag = 'vec_tag_v'
  []
  [extra_matrix_v]
    type = TagMatrixAux
    variable = matrix_v_diag
    v = v
    matrix_tag = 'mat_tag_v'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'LINEAR'
  system_names = 'u_sys v_sys'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  execute_on = timestep_end
[]
