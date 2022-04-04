[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  parallel_type = replicated
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./v]
    initial_condition = 1
  [../]
  [./inverse_v]
    initial_condition = 1
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
  [./force_u]
    type = CoupledForce
    variable = u
    v = inverse_v
  [../]
[]

[AuxKernels]
  [./invert_v]
    type = QuotientAux
    variable = inverse_v
    denominator = v
    numerator = 20.0
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./Neumann_right]
    type = NeumannBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Postprocessors]
  [picard_its]
    type = NumFixedPointIterations
    execute_on = 'initial timestep_end'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 4
  dt = 0.5
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  fixed_point_max_its = 30
  nl_abs_tol = 1e-14
  relaxation_factor = 2.0
  transformed_variables = u
[]

[Outputs]
  exodus = true
  execute_on = 'INITIAL TIMESTEP_END'
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    execute_on = timestep_begin
    positions = '0 0 0'
    input_files = picard_relaxed_sub.i
  [../]
[]

[Transfers]
  [./v_from_sub]
    type = MultiAppNearestNodeTransfer
    from_multi_app = sub
    source_variable = v
    variable = v
  [../]
  [./u_to_sub]
    type = MultiAppNearestNodeTransfer
    to_multi_app = sub
    source_variable = u
    variable = u
  [../]
[]
