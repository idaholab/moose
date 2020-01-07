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
  [lm]
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
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
  [ffn]
    type = BodyForce
    variable = u
    function = '1'
  []
[]

[NodalKernels]
  [positive_constraint]
    type = UpperBoundNodalKernel
    variable = lm
    v = u
    exclude_boundaries = 'left right'
    upper_bound = 10
  []
  [forces]
    type = CoupledForceNodalKernel
    variable = u
    v = lm
    coef = -1
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
  [active_lm]
    type = GreaterThanLessThanPostprocessor
    variable = lm
    execute_on = 'nonlinear timestep_end'
    value = 1e-8
  []
  [violations]
    type = GreaterThanLessThanPostprocessor
    variable = u
    execute_on = 'nonlinear timestep_end'
    value = ${fparse 10+1e-8}
    comparator = 'greater'
  []
[]
