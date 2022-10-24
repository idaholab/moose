[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [u]
  []
[]

[AuxKernels]
  [pid_aux]
    type = ProcessorIDAux
    variable = pid
    execute_on = 'INITIAL'
  []
[]

[AuxVariables]
  [pid]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
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

[AuxVariables]
  [from_sub]
  []
  [elemental_from_sub]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 1

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = sub_diffusion.i
    execute_on = timestep_end
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppGeneralFieldNearestNodeTransfer
    source_variable = u
    variable = transferred_u
    to_multi_app = sub
  []

  [elemental_to_sub]
    type = MultiAppGeneralFieldNearestNodeTransfer
    source_variable = u
    variable = elemental_transferred_u
    to_multi_app = sub
  []

  [from_sub]
    type = MultiAppGeneralFieldNearestNodeTransfer
    source_variable = sub_u
    variable = from_sub
    from_multi_app = sub
  []

  [elemental_from_sub]
    type = MultiAppGeneralFieldNearestNodeTransfer
    source_variable = sub_u
    variable = elemental_from_sub
    from_multi_app = sub
  []
[]
