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

[AuxVariables]
  [v][]
  [v_elem]
    order = CONSTANT
    family = MONOMIAL
  []
  [w][]
  [w_elem]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[ICs]
  [w]
    type = FunctionIC
    function = 'cos(x)*sin(y)'
    variable = w
  []
  [w_elem]
    type = FunctionIC
    function = 'cos(x)*sin(y)'
    variable = w_elem
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = CoupledForce
    variable = u
    v = v
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
    type = FullSolveMultiApp
    app_type = MooseTestApp
    positions = '0 0 0'
    input_files = 'sub-app.i'
    execute_on = 'timestep_begin'
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppCopyTransfer
    from_multi_app = sub
    source_variable = v
    variable = v
    execute_on = 'timestep_begin'
  []
  [from_sub_elem]
    type = MultiAppCopyTransfer
    from_multi_app = sub
    source_variable = v_elem
    variable = v_elem
    execute_on = 'timestep_begin'
  []
  [to_sub]
    type = MultiAppCopyTransfer
    to_multi_app = sub
    source_variable = w
    variable = w
    execute_on = 'timestep_begin'
  []
  [to_sub_elem]
    type = MultiAppCopyTransfer
    to_multi_app = sub
    source_variable = w_elem
    variable = w_elem
    execute_on = 'timestep_begin'
  []
[]
