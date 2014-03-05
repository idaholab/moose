[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD9
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    value = 'x*x+y*y'
  [../]

  [./ffn]
    type = ParsedFunction
    value = -4
  [../]

  [./bottom_bc_fn]
    type = ParsedFunction
    value = -2*y
  [../]

  [./right_bc_fn]
    type = ParsedFunction
    value =  2*x
  [../]

  [./top_bc_fn]
    type = ParsedFunction
    value =  2*y
  [../]

  [./left_bc_fn]
    type = ParsedFunction
    value = -2*x
  [../]
[]

# NL

[Variables]
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]

  [./lambda]
    family = SCALAR
    order = FIRST
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffnk]
    type = UserForcingFunction
    variable = u
    function = ffn
  [../]

  [./sk_lm]
    type = ScalarLagrangeMultiplier
    variable = u
    lambda = lambda
  [../]
[]

[ScalarKernels]
  [./constraint]
    type = PostprocessorCED
    variable = lambda
    pp_name = pp
    value = 2.666666666666666
  [../]
[]

[BCs]
  [./bottom]
    type = FunctionNeumannBC
    variable = u
    boundary = '0'
    function = bottom_bc_fn
  [../]
  [./right]
    type = FunctionNeumannBC
    variable = u
    boundary = '1'
    function = right_bc_fn
  [../]
  [./top]
    type = FunctionNeumannBC
    variable = u
    boundary = '2'
    function = top_bc_fn
  [../]
  [./left]
    type = FunctionNeumannBC
    variable = u
    boundary = '3'
    function = left_bc_fn
  [../]
[]

[Postprocessors]
  [./pp]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = residual
  [../]
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Preconditioning]
  active = 'pc'

  [./pc]
    type = SMP
    full = true
    solve_type = 'PJFNK'
  [../]
[] # End preconditioning block

[Executioner]
  type = Steady
  nl_rel_tol = 1e-15
[]

[Outputs]
  output_initial = true
  exodus = true
  hide = 'lambda'
  [./console]
    type = Console
    perf_log = true
  [../]
[]
