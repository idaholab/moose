[Mesh]
  [mesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0.0
    xmax = 7.0
    nx = 3
  []
[]

[Variables]
  [temp]
    initial_condition = 300
  []
[]

[Kernels]
  [time]
    type = CoefTimeDerivative
    variable = temp
    Coefficient = '${fparse 1.00630182*1.225}'
  []
  [heat_conduc]
    type = MatDiffusion
    variable = temp
    diffusivity = 'k'
  []
[]

[BCs]
  [dirichlet]
    type = FunctionDirichletBC
    function = "200"
    variable = temp
    boundary = 'right'
  []
[]

[Functions]
  [design_function]
    type = ParsedFunction
    value = 't/3600*297'
  []
  [reward_function]
    type = ScaledAbsDifferenceDRLRewardFunction
    design_function = design_function
    observed_value = center_temp_tend
    c1 = 1
    c2 = 10
  []
[]

[Materials]
  [constant]
    type = GenericConstantMaterial
    prop_names = 'k'
    prop_values = 26.53832364
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'
  line_search = 'none'

  nl_rel_tol = 1e-8

  start_time = 0.0
  end_time = 3600
  dt = 1800
[]

[Postprocessors]
  [center_temp_tend]
    type = PointValue
    variable = temp
    point = '3.5 0.0 0.0'
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [reward]
    type = FunctionValuePostprocessor
    function = reward_function
    execute_on = 'INITIAL TIMESTEP_END'
    indirect_dependencies = 'center_temp_tend'
  []
[]

[Outputs]
  csv = true
[]
