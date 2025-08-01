l=10
nx=100
num_steps=${l}
dt=1

[GlobalParams]
  lm_sign_positive = false
[]

[Problem]
  extra_tag_vectors = 'positive diffusion rest'
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmax = ${l}
  nx = ${nx}
  elem_type = EDGE3
[]

[Variables]
  [u]
    order = SECOND
  []
  [lm]
  []
[]

[AuxVariables]
  [positive][]
  [diffusion_lm][]
  [rest_lm][]
  [diffusion_primal]
    order = SECOND
  []
  [rest_primal]
    order = SECOND
  []
[]

[AuxKernels]
  [positive]
    type = TagVectorAux
    variable = positive
    v = lm
    vector_tag = positive
  []
  [diffusion_lm]
    type = TagVectorAux
    variable = diffusion_lm
    v = lm
    vector_tag = diffusion
  []
  [rest_lm]
    type = TagVectorAux
    variable = rest_lm
    v = lm
    vector_tag = rest
  []
  [diffusion_primal]
    type = TagVectorAux
    variable = diffusion_primal
    v = u
    vector_tag = diffusion
  []
  [rest_primal]
    type = TagVectorAux
    variable = rest_primal
    v = u
    vector_tag = rest
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = '${l} - x'
  []
[]

[Kernels]
  [time]
    type = TimeDerivativeLM
    variable = u
    lm_variable = lm
    extra_vector_tags = 'rest'
  []
  [diff]
    type = Diffusion
    variable = u
    extra_vector_tags = 'diffusion'
  []
  [diff_lm]
    type = LMDiffusion
    variable = lm
    primal_variable = u
    extra_vector_tags = 'diffusion'
  []
  [ffn]
    type = BodyForceLM
    variable = u
    lm_variable = lm
    function = '-1'
    extra_vector_tags = 'rest'
  []
  [lm_coupled_force]
    type = CoupledForceLM
    variable = u
    v = lm
    lm_variable = lm
    extra_vector_tags = 'rest'
  []
[]

[NodalKernels]
  [positive_constraint]
    type = LowerBoundNodalKernel
    extra_vector_tags = positive
    variable = lm
    v = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    boundary = left
    value = ${l}
    variable = u
  []
  [right]
    type = DirichletBC
    boundary = right
    value = 0
    variable = u
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  num_steps = ${num_steps}
  dt = ${dt}
  dtmin = ${dt}
  solve_type = NEWTON
  petsc_options_iname = '-snes_max_linear_solve_fail -ksp_max_it -pc_factor_levels -ksp_gmres_restart'
  petsc_options_value = '0                           99          16                99'
[]

[Outputs]
  exodus = true
  csv = true
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]

[Postprocessors]
  [active_lm]
    type = GreaterThanLessThanPostprocessor
    variable = lm
    execute_on = 'nonlinear timestep_end'
    value = 1e-12
  []
  [violations]
    type = GreaterThanLessThanPostprocessor
    variable = u
    execute_on = 'nonlinear timestep_end'
    value = -1e-12
    comparator = 'less'
  []
[]

[Debug]
  show_var_residual_norms = true
[]
