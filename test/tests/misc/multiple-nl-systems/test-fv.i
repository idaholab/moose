[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[Problem]
  nl_sys_names = 'u v'
  error_on_jacobian_nonzero_reallocation = true
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    nl_sys = 'u'
  []
  [v]
    type = MooseVariableFVReal
    nl_sys = 'v'
  []
[]

[FVKernels]
  [diff_u]
    type = FVDiffusion
    variable = u
    coeff = 1.0
  []
  [diff_v]
    type = FVDiffusion
    variable = v
    coeff = 1.0
  []
  [force]
    type = FVCoupledForce
    variable = v
    v = u
  []
[]

[FVBCs]
  [left_u]
    type = FVDirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right_u]
    type = FVDirichletBC
    variable = u
    boundary = right
    value = 1
  []
  [left_v]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 0
  []
  [right_v]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = SteadySolve2
  solve_type = 'NEWTON'
  petsc_options = '-snes_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  first_nl_sys_to_solve = 'u'
  second_nl_sys_to_solve = 'v'
[]

[Functions]
  [exact]
    type = ParsedFunction
    value = '-1/6*x*x*x +7/6*x'
  []
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    function = exact
    variable = v
    execute_on = FINAL
    outputs = 'csv'
  []
  [h]
    type = AverageElementSize
    outputs = 'console csv'
    execute_on = FINAL
  []
[]

[Outputs]
  print_nonlinear_residuals = false
  print_linear_residuals = false
  exodus = true
  [csv]
    type = CSV
    execute_on = 'FINAL'
  []
[]
