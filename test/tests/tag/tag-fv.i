[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Variables]
  [v]
    type = MooseVariableFVReal
    initial_condition = 7
  []
[]

[AuxVariables]
  [soln_dof]
    type = MooseVariableFVReal
  []
  [soln_old_dof]
    type = MooseVariableFVReal
  []
  [soln_older_dof]
    type = MooseVariableFVReal
  []
  [resid_nontime_dof]
    type = MooseVariableFVReal
  []
  [soln]
    type = MooseVariableFVReal
  []
  [soln_old]
    type = MooseVariableFVReal
  []
  [soln_older]
    type = MooseVariableFVReal
  []
  [resid_nontime]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [soln_dof]
    type = TagVectorDofValueAux
    variable = soln_dof
    v = v
    vector_tag = 'solution'
  []
  [soln_old_dof]
    type = TagVectorDofValueAux
    variable = soln_old_dof
    v = v
    vector_tag = 'solution_state_1'
  []
  [soln_older_dof]
    type = TagVectorDofValueAux
    variable = soln_older_dof
    v = v
    vector_tag = 'solution_state_2'
  []
  [nontime_dof]
    type = TagVectorDofValueAux
    variable = resid_nontime_dof
    v = v
    vector_tag = 'nontime'
  []
  [soln]
    type = TagVectorAux
    variable = soln
    v = v
    vector_tag = 'solution'
  []
  [soln_old]
    type = TagVectorAux
    variable = soln_old
    v = v
    vector_tag = 'solution_state_1'
  []
  [soln_older]
    type = TagVectorAux
    variable = soln_older
    v = v
    vector_tag = 'solution_state_2'
  []
  [nontime]
    type = TagVectorAux
    variable = resid_nontime
    v = v
    vector_tag = 'nontime'
  []
[]

[FVKernels]
  [time]
    type = FVTimeKernel
    variable = v
  []
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 7
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 42
  []
[]

[Materials]
  [diff]
    type = ADGenericFunctorMaterial
    prop_names = 'coeff'
    prop_values = '.2'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  num_steps = 5
  dt = 0.1
[]

[Outputs]
  exodus = true
[]
