#
# This test is identical to scalar_constraint_kernel.i, but it everything is evaluated on the displaced mesh
#

[GlobalParams]
  use_displaced_mesh = true
[]

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
  displacements = 'disp_x disp_y'
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = 'x*x+y*y'
  [../]

  [./ffn]
    type = ParsedFunction
    expression = -4
  [../]

  [./bottom_bc_fn]
    type = ParsedFunction
    expression = -2*y
  [../]

  [./right_bc_fn]
    type = ParsedFunction
    expression =  2*x
  [../]

  [./top_bc_fn]
    type = ParsedFunction
    expression =  2*y
  [../]

  [./left_bc_fn]
    type = ParsedFunction
    expression = -2*x
  [../]
[]

[AuxVariables]
  [./disp_x]
    family = LAGRANGE
    order = SECOND
  [../]
  [./disp_y]
    family = LAGRANGE
    order = SECOND
  [../]
[]

[AuxKernels]
  [./disp_x_ak]
    type = ConstantAux
    variable = disp_x
    value = 0
  [../]
  [./disp_y_ak]
    type = ConstantAux
    variable = disp_y
    value = 0
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
    type = BodyForce
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
    type = AverageValueConstraint
    variable = lambda
    pp_name = pp
    value = 2.666666666666666
    # overrride the global setting, scalar kernels do not live on a mesh
    use_displaced_mesh = false
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
    execute_on = linear
  [../]
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
    execute_on = 'initial timestep_end'
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
    solve_type = 'PJFNK'
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-14
  l_tol = 1e-7
[]

[Outputs]
  exodus = true
  hide = lambda
[]
