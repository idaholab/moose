[Mesh]
  # This test uses SolutionUserObject which doesn't work with DistributedMesh.
  type = FileMesh
  file = aux_nonlinear_solution_adapt_out_0004_mesh.xda
  parallel_type = replicated
[]

[Adaptivity]
  marker = error_frac
  steps = 2
  [./Indicators]
    [./jump_indicator]
      type = GradientJumpIndicator
      variable = u
    [../]
  [../]
  [./Markers]
    [./error_frac]
      type = ErrorFractionMarker
      indicator = jump_indicator
      refine = 0.7
    [../]
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./u_aux]
  [../]
[]

[Functions]
  [./u_xda_func]
    type = SolutionFunction
    solution = xda_u
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./aux_xda_kernel]
    type = SolutionAux
    variable = u_aux
    solution = xda_u_aux
    execute_on = initial
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 2
  [../]
[]

[UserObjects]
  [./xda_u_aux]
    type = SolutionUserObject
    system = aux0
    mesh = aux_nonlinear_solution_adapt_out_0004_mesh.xda
    es = aux_nonlinear_solution_adapt_out_0004.xda
    system_variables = u_aux
    execute_on = initial
  [../]
  [./xda_u]
    type = SolutionUserObject
    system = nl0
    mesh = aux_nonlinear_solution_adapt_out_0004_mesh.xda
    es = aux_nonlinear_solution_adapt_out_0004.xda
    system_variables = u
    execute_on = initial
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
[]

[ICs]
  [./u_func_ic]
    function = u_xda_func
    variable = u
    type = FunctionIC
  [../]
[]
