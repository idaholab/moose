l=10
nx=80
num_steps=2

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmax = ${l}
  nx = ${nx}
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [bounds][]
[]

[Bounds]
  [./u_upper_bounds]
    type = ConstantBoundsAux
    variable = bounds
    bounded_variable = u
    bound_type = upper
    bound_value = ${l}
  [../]
  [./u_lower_bounds]
    type = ConstantBoundsAux
    variable = bounds
    bounded_variable = u
    bound_type = lower
    bound_value = 0
  [../]
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
    function = 'if(x<5,-1,1)'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    boundary = left
    value = 0
    variable = u
  []
  [right]
    type = DirichletBC
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
  petsc_options = '-snes_vi_monitor'
  petsc_options_iname = '-snes_max_linear_solve_fail -ksp_max_it -pc_type -sub_pc_factor_levels -snes_linesearch_type -snes_type'
  petsc_options_value = '0                           30          asm      16                    basic                 vinewtonrsls'
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
  active = 'upper_violations lower_violations'
  [upper_violations]
    type = GreaterThanLessThanPostprocessor
    variable = u
    execute_on = 'nonlinear timestep_end'
    value = ${fparse 10+1e-8}
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
