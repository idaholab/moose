[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8
  uniform_refine = 2
[]

[Variables]
  [./phi]
  [../]
[]

[AuxVariables]
  [./phi_0]
    family = MONOMIAL
    order = FIRST
  [../]
  [./phi_exact]
  [../]
[]

[AuxKernels]
  [./phi_exact]
    type = FunctionAux
    function = phi_exact
    variable = phi_exact
  [../]
[]

[Functions]
  [./phi_initial]
    type = ParsedFunction
    expression = '1-x'
  [../]
  [./phi_exact]
    type = ParsedFunction
    symbol_names = epsilon
    symbol_values = 0.05
    expression = '1 / (1+exp((x-0.5)/epsilon))'
  [../]
[]

[ICs]
  [./phi_ic]
    type = FunctionIC
    function = phi_initial
    variable = phi
  [../]
  [./phi_0_ic]
    type = FunctionIC
    function = phi_initial
    variable = phi_0
  [../]
[]

[Kernels]
  [./time]
    type = TimeDerivative
    variable = phi
  [../]

  [./reinit]
    type = LevelSetOlssonReinitialization
    variable = phi
    phi_0 = phi_0
    epsilon = 0.05
  [../]
[]

[UserObjects]
  [./arnold]
    type = LevelSetOlssonTerminator
    tol = 0.1
  [../]
[]

[Postprocessors]
  [./error]
    type = ElementL2Error
    variable = phi
    function = phi_exact
    execute_on = 'initial timestep_end'
  [../]
  [./ndofs]
    type = NumDOFs
  [../]
[]

[VectorPostprocessors]
  [./line]
    type = LineValueSampler
    start_point = '0 0.5 0'
    end_point =  '1 0.5 0'
    variable = phi
    num_points = 100
    sort_by = x
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  l_max_its = 100
  nl_max_its = 100
  solve_type = PJFNK
  num_steps = 10
  start_time = 0
  nl_abs_tol = 1e-13
  scheme = implicit-euler
  dt = 0.05
  petsc_options_iname = '-pc_type -pc_sub_type -ksp_gmres_restart'
  petsc_options_value = 'hypre    boomeramg    300'
[]

[Outputs]
  exodus = true
  [./out]
    type = CSV
    time_data = true
    file_base = output/olsson_1d_out
  [../]
[]
