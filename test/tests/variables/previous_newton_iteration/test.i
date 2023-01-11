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

[Problem]
  previous_nl_solution_required = true
[]

[Functions]
  [./v_fn]
    type = ParsedFunction
    expression = -4+(x*x+y*y)+1
  [../]

  [./left_u_bc_fn]
    type = ParsedFunction
    expression = -2*x
  [../]
  [./top_u_bc_fn]
    type = ParsedFunction
    expression = 2*y
  [../]
  [./right_u_bc_fn]
    type = ParsedFunction
    expression = 2*x
  [../]
  [./bottom_u_bc_fn]
    type = ParsedFunction
    expression = -2*y
  [../]
[]

[AuxVariables]
  [./a]
    order = SECOND
  [../]
  [./v]
    order = SECOND
  [../]
[]

[AuxKernels]
  [./ak_a]
    type = QuotientAux
    variable = a
    numerator = v
    denominator = u
  [../]

  [./ak_v]
    type = FunctionAux
    variable = v
    function = v_fn
  [../]
[]

[Variables]
  [./u]
    order = SECOND
  [../]
[]

[ICs]
  [./u_ic]
    type = ConstantIC
    variable = u
    value = 1
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./react]
    type = Reaction
    variable = u
  [../]
  [./cv_u]
    type = CoupledForceLagged
    variable = u
    v = v
  [../]
[]

[BCs]
  [./u_bc_left]
    type = FunctionNeumannBC
    variable = u
    boundary = 'left'
    function = left_u_bc_fn
  [../]

  [./u_bc_top]
    type = FunctionNeumannBC
    variable = u
    boundary = 'top'
    function = top_u_bc_fn
  [../]

  [./u_bc_right]
    type = FunctionNeumannBC
    variable = u
    boundary = 'right'
    function = right_u_bc_fn
  [../]

  [./u_bc_bottom]
    type = FunctionNeumannBC
    variable = u
    boundary = 'bottom'
    function = bottom_u_bc_fn
  [../]
[]

[Preconditioning]
  [./pc]
    type = SMP
    full = true
    solve_type = PJFNK
  [../]
[]

[Executioner]
  type = Steady

  # to get multiple NL iterations
  l_tol = 1e-3
  nl_rel_tol = 1e-10
[]

[Outputs]
  [./out]
    type = Exodus
    execute_on = 'nonlinear'
  [../]
[]
