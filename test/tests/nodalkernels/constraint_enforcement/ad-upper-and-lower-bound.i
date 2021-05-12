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

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = 'x'
  []
[]

[Kernels]
  [time]
    type = ADTimeDerivative
    variable = u
  []
  [diff]
    type = ADDiffusion
    variable = u
  []
  [ffn]
    type = ADBodyForce
    variable = u
    function = 'if(x<5,-1,1)'
  []
[]

[NodalKernels]
  [upper_bound]
    type = ADUpperBoundNodalKernel
    variable = lm_upper
    v = u
    exclude_boundaries = 'left right'
    upper_bound = 10
  []
  [forces_from_upper]
    type = ADCoupledForceNodalKernel
    variable = u
    v = lm_upper
    coef = -1
  []
  [lower_bound]
    type = ADLowerBoundNodalKernel
    variable = lm_lower
    v = u
    exclude_boundaries = 'left right'
    lower_bound = 0
  []
  [forces_from_lower]
    type = ADCoupledForceNodalKernel
    variable = u
    v = lm_lower
    coef = 1
  []
[]


[BCs]
  [left]
    type = ADDirichletBC
    boundary = left
    value = 0
    variable = u
  []
  [right]
    type = ADDirichletBC
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
