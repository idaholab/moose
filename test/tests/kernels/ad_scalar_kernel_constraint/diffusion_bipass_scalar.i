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
  [exact_fn]
    type = ParsedFunction
    value = 'x*x+y*y'
  []

  [ffn]
    type = ParsedFunction
    value = -4
  []

  [bottom_bc_fn]
    type = ParsedFunction
    value = -2*y
  []

  [right_bc_fn]
    type = ParsedFunction
    value =  2*x
  []

  [top_bc_fn]
    type = ParsedFunction
    value =  2*y
  []

  [left_bc_fn]
    type = ParsedFunction
    value = -2*x
  []
[]

[Variables]
  [u]
    family = LAGRANGE
    order = SECOND
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[Kernels]
  # Make sure that we can derive from the scalar base class
  # but actually not assign a scalar variable
  [diff]
    type = ADDiffusionNoScalar
    variable = u
  []

  [ffnk]
    type = ADBodyForce
    variable = u
    function = ffn
  []

  [sk_lm]
    type = ADScalarLMKernel
    variable = u
    kappa = lambda
    pp_name = pp
    value = 2.666666666666666
  []
[]

[Problem]
  kernel_coverage_check = false
  error_on_jacobian_nonzero_reallocation = true
[]

[BCs]
  [bottom]
    type = ADFunctionNeumannBC
    variable = u
    boundary = 'bottom'
    function = bottom_bc_fn
  []
  [right]
    type = ADFunctionNeumannBC
    variable = u
    boundary = 'right'
    function = right_bc_fn
  []
  [top]
    type = ADFunctionNeumannBC
    variable = u
    boundary = 'top'
    function = top_bc_fn
  []
  [left]
    type = ADFunctionNeumannBC
    variable = u
    boundary = 'left'
    function = left_bc_fn
  []
[]

[Postprocessors]
  # integrate the volume of domain since original objects set
  # int(phi)=V0, rather than int(phi-V0)=0
  [pp]
    type = FunctionElementIntegral
    function = 1
    execute_on = initial
  []
  [l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
    execute_on = 'initial timestep_end'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  []
[]

[Executioner]
  type = Steady
  residual_and_jacobian_together = true
  nl_rel_tol = 1e-9
  l_tol = 1.e-10
  nl_max_its = 10
  # This example builds an indefinite matrix, so "-pc_type hypre -pc_hypre_type boomeramg" cannot
  # be used reliably on this problem
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  # This is a linear problem, so we don't need to recompute the
  # Jacobian. This isn't a big deal for a Steady problems, however, as
  # there is only one solve.
  solve_type = 'LINEAR'
[]

[Outputs]
#  exodus = true
  csv = true
  hide = lambda
[]
