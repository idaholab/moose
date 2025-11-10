l=10
nx=100
num_steps=10

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmax = ${l}
  nx = ${nx}
[]

[Variables]
  [u]
  []
  [lm_upper]
  []
  [lm_lower]
  []
[]

[AuxVariables]
  [force]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = 'x'
  []
  [force]
    type = FunctionIC
    variable = force
    function = 'if(x<5,-1,1)'
  []
[]

[Kernels]
  [time]
    type = KokkosTimeDerivative
    variable = u
  []
  [diff]
    type = KokkosDiffusion
    variable = u
  []
  [ffn]
    type = KokkosCoupledForce
    variable = u
    v = force
  []
[]

[NodalKernels]
  [upper_bound]
    type = KokkosUpperBoundNodalKernel
    variable = lm_upper
    v = u
    exclude_boundaries = 'left right'
    upper_bound = 10
  []
  [forces_from_upper]
    type = KokkosCoupledForceNodalKernel
    variable = u
    v = lm_upper
    coef = -1
  []
  [lower_bound]
    type = KokkosLowerBoundNodalKernel
    variable = lm_lower
    v = u
    exclude_boundaries = 'left right'
    lower_bound = 0
  []
  [forces_from_lower]
    type = KokkosCoupledForceNodalKernel
    variable = u
    v = lm_lower
    coef = 1
  []
[]


[BCs]
  [left]
    type = KokkosDirichletBC
    boundary = left
    value = 0
    variable = u
  []
  [right]
    type = KokkosDirichletBC
    boundary = right
    value = ${l}
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
  solve_type = NEWTON
  dtmin = 1
  petsc_options_iname = '-snes_max_linear_solve_fail -ksp_max_it -pc_type -sub_pc_factor_levels -snes_linesearch_type'
  petsc_options_value = '0                           30          asm      16                    basic'
[]

[Outputs]
  exodus = true
  hide = force
  [csv]
    type = CSV
    execute_on = 'nonlinear timestep_end'
  []
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Postprocessors]
  [active_upper_lm]
    type = GreaterThanLessThanPostprocessor
    variable = lm_upper
    execute_on = 'nonlinear timestep_end'
    value = 1e-8
    comparator = 'greater'
  []
  [upper_violations]
    type = GreaterThanLessThanPostprocessor
    variable = u
    execute_on = 'nonlinear timestep_end'
    value = ${fparse 10+1e-8}
    comparator = 'greater'
  []
  [active_lower_lm]
    type = GreaterThanLessThanPostprocessor
    variable = lm_lower
    execute_on = 'nonlinear timestep_end'
    value = 1e-8
    comparator = 'greater'
  []
  [lower_violations]
    type = GreaterThanLessThanPostprocessor
    variable = u
    execute_on = 'nonlinear timestep_end'
    value = -1e-8
    comparator = 'less'
  []
  [nls]
    type = NumNonlinearIterations
  []
  [cum_nls]
    type = CumulativeValuePostprocessor
    postprocessor = nls
  []
[]
