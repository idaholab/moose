[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [u]
    components = 2
  []
[]

[AuxVariables]
  [v]
    components = 2
    initial_condition = '1 1'
  []
  [inverse_v]
    components = 2
    initial_condition = '1 1'
  []
[]

[Kernels]
  [diff]
    type = ArrayDiffusion
    variable = u
    diffusion_coefficient = dc
  []
  [time]
    type = ArrayTimeDerivative
    variable = u
    time_derivative_coefficient = tc
  []
  [force_u]
    type = ArrayCoupledForce
    variable = u
    v = inverse_v
    is_v_array = true
    coef = '1 1'
  []
[]

[AuxKernels]
  [invert_v]
    type = ArrayQuotientAux
    variable = inverse_v
    denominator = v
    numerator = '20 20'
  []
[]

[BCs]
  [left]
    type = ArrayDirichletBC
    variable = u
    boundary = left
    values = '0 0'
  []
  [Neumann_right]
    type = ArrayNeumannBC
    variable = u
    boundary = right
    value = '1 1'
  []
[]

[Materials]
  [dc]
    type = GenericConstantArray
    prop_name = dc
    prop_value = '0.1 0.1'
  []
  [tc]
    type = GenericConstantArray
    prop_name = tc
    prop_value = '1 1'
  []
[]

[Postprocessors]
  [picard_its]
    type = NumPicardIterations
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
  dt = 0.5
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  picard_max_its = 30
  nl_abs_tol = 1e-14
  relaxation_factor = 0.8
  relaxed_variables = u
[]

[Outputs]
  exodus = true
  execute_on = 'INITIAL TIMESTEP_END'
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    execute_on = timestep_begin
    input_files = picard_relaxed_array_sub.i
  []
[]

[Transfers]
  [v_from_sub]
    type = MultiAppCopyTransfer
    from_multi_app = sub
    source_variable = v
    variable = v
  []
  [u_to_sub]
    type = MultiAppCopyTransfer
    to_multi_app = sub
    source_variable = u
    variable = u
  []
[]
