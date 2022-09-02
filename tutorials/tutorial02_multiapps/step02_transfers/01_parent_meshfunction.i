[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [tv]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
  [force]
    type = BodyForce
    variable = u
    value = 1.
  []
  [td]
    type = TimeDerivative
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
    value = 0
  []
[]

[Executioner]
  type = Transient
  end_time = 2
  dt = 0.2

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    positions = '0 0 0'
    input_files = '01_sub_meshfunction.i'
  []
[]

[Transfers]
  [pull_v]
    type = MultiAppShapeEvaluationTransfer

    # Transfer from the sub-app to this app
    from_multi_app = sub_app

    # The name of the variable in the sub-app
    source_variable = v

    # The name of the auxiliary variable in this app
    variable = tv
  []

  [push_u]
    type = MultiAppShapeEvaluationTransfer

    # Transfer to the sub-app from this app
    to_multi_app = sub_app

    # The name of the variable in this app
    source_variable = u

    # The name of the auxiliary variable in the sub-app
    variable = tu
  []
[]
