[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [time]
    type = TimeDerivative
    variable = u
  []
[]

[BCs]
  [left]
    type = FunctionDirichletBC
    variable = u
    boundary = left
    function = 'if(t<1.0,t,1.0)'
  []
  [right]
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = 'if(t<1.0,2.0-t,1.0)'
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'

[]

[Postprocessors]
  active = 'max_nl_dofs nl_dofs'
  [max_nl_dofs]
    type = TimeExtremeValue
    value_type = max
    postprocessor = nl_dofs
    execute_on = 'initial timestep_end'
  []
  [time_of_max_nl_dofs]
    type = TimeExtremeValue
    value_type = max
    output_type = time
    postprocessor = nl_dofs
    execute_on = 'initial timestep_end'
  []
  [nl_dofs]
    type = NumDOFs
    system = NL
    execute_on = 'initial timestep_end'
  []
[]

[Adaptivity]
  marker = marker
  max_h_level = 2
  [Markers]
    [marker]
      type = ValueRangeMarker
      lower_bound = 0.7
      upper_bound = 1.3
      buffer_size = 0.2
      variable = u
      invert = true
      third_state = DO_NOTHING
    []
  []
[]

[Outputs]
  csv = true
[]
