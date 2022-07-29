[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = -1
  ymax = 0
  nx = 10
  ny = 10
  alpha_rotation = 90
[]

[Variables]
  [u][]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = CoupledForce
    variable = u
    v = new_val_x
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  verbose = true
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = CentroidMultiApp
    app_type = MooseTestApp
    input_files = 'sub-app.i'
    execute_on = 'timestep_begin'
  []
[]

[Transfers]
  [send]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    to_multi_app = sub
    source_variable = x_nodal
    postprocessor = rec_x
  []
  [send_elem]
    type = MultiAppVariableValueSamplePostprocessorTransfer
    to_multi_app = sub
    source_variable = y_elem
    postprocessor = rec_y
  []

  [get_back]
    type = MultiAppPostprocessorInterpolationTransfer
    from_multi_app = sub
    variable = new_val_x
    postprocessor = rec_x
  []
  [get_back_elem]
    type = MultiAppPostprocessorInterpolationTransfer
    from_multi_app = sub
    variable = new_val_y_elem
    postprocessor = rec_y
  []
[]

[AuxVariables]
  [x_nodal]
    [InitialCondition]
      type = FunctionIC
      function = 'x'
    []
  []
  [y_elem]
    order = CONSTANT
    family = MONOMIAL
    [InitialCondition]
      type = FunctionIC
      function = 'y'
    []
  []
  [new_val_x]
  []
  [new_val_y_elem]
    order = CONSTANT
    family = MONOMIAL
  []
[]
