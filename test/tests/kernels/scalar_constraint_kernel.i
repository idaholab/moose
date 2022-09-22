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
    type = ScalarLMKernel
    variable = u
    coupled_scalar = lambda
    kappa = lambda
    pp_name = pp
    value = 2.666666666666666
  [../]
[]

# This seems to be needed so that the scalar variable is 
# 'detected' and added to the system. Otherwise this message: 
# *** ERROR ***
# Variable 'scalar_variable' does not exist in this system
[ScalarKernels]
  [./null]
    type = NullScalarKernel
    variable = lambda
  [../]
[]

# With above block, not needed, and results in Petsc error messages
# [Problem]
#   kernel_coverage_check = false
#   error_on_jacobian_nonzero_reallocation = true
# []

[BCs]
  [./bottom]
    type = FunctionNeumannBC
    variable = u
    boundary = 'bottom'
    function = bottom_bc_fn
  [../]
  [./right]
    type = FunctionNeumannBC
    variable = u
    boundary = 'right'
    function = right_bc_fn
  [../]
  [./top]
    type = FunctionNeumannBC
    variable = u
    boundary = 'top'
    function = top_bc_fn
  [../]
  [./left]
    type = FunctionNeumannBC
    variable = u
    boundary = 'left'
    function = left_bc_fn
  [../]
[]

[Postprocessors]
  # integrate the volume of domain since original objects set 
  # int(phi)=V0, rather than int(phi-V0)=0
  [./pp]
    type = FunctionElementIntegral
    function = 1
    execute_on = initial
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
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-9
  l_tol = 1.e-10
  nl_max_its = 10
  # This example builds an indefinite matrix, so "-pc_type hypre -pc_hypre_type boomeramg" cannot
  # be used reliably on this problem. ILU(0) seems to do OK in both serial and parallel in my testing,
  # I have not seen any zero pivot issues.
  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'bjacobi  ilu'
  # This is a linear problem, so we don't need to recompute the
  # Jacobian. This isn't a big deal for a Steady problems, however, as
  # there is only one solve.
  solve_type = 'LINEAR'
[]

[Outputs]
  exodus = true
  hide = lambda
[]
