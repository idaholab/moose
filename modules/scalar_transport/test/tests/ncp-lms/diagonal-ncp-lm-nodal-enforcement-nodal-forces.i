l = 10
nx = 100
num_steps = ${l}
dt = 1

[GlobalParams]
  lm_sign_positive = false
[]

[Problem]
  extra_tag_residuals = 'positive diffusion rest'
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
    order = SECOND
  []
[]

[AuxVariables]
  [positive]
    order = SECOND
  []
  [diffusion_lm]
    order = SECOND
  []
  [rest_lm]
    order = SECOND
  []
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
    residual_tag = positive
  []
  [diffusion_lm]
    type = TagVectorAux
    variable = diffusion_lm
    v = lm
    residual_tag = diffusion
  []
  [rest_lm]
    type = TagVectorAux
    variable = rest_lm
    v = lm
    residual_tag = rest
  []
  [diffusion_primal]
    type = TagVectorAux
    variable = diffusion_primal
    v = u
    residual_tag = diffusion
  []
  [rest_primal]
    type = TagVectorAux
    variable = rest_primal
    v = u
    residual_tag = rest
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
    extra_residual_tags = 'rest'
  []
  [diff]
    type = Diffusion
    variable = u
    extra_residual_tags = 'diffusion'
  []
  [diff_lm]
    type = LMDiffusion
    variable = lm
    primal_variable = u
    extra_residual_tags = 'diffusion'
  []
  [ffn]
    type = BodyForceLM
    variable = u
    lm_variable = lm
    function = '-1'
    extra_residual_tags = 'rest'
  []
[]

[NodalKernels]
  [forces]
    type = CoupledForceNodalKernel
    variable = u
    v = lm
    extra_residual_tags = 'rest'
  []
  [corresponding_lm_portion]
    type = ReactionNodalKernel
    variable = lm
    coeff = 1
    extra_residual_tags = 'rest'
  []
  [positive_constraint]
    type = LowerBoundNodalKernel
    extra_residual_tags = positive
    variable = lm
    v = u
    # exclude_boundaries = 'left right'
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
  petsc_options_iname = '-snes_max_linear_solve_fail -ksp_max_it -pc_factor_levels'
  petsc_options_value = '0                           30          16'
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
