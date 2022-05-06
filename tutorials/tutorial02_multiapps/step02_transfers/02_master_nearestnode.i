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
    value = 1
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
    positions = '0.1 0.1 0  0.4 0.4 0  0.7 0.7 0'
    input_files = '02_sub_nearestnode.i'
    execute_on = timestep_end
    output_in_position = true
  []
[]

[Transfers]
  [push_u]
    type = MultiAppNearestNodeTransfer

    # Transfer to the sub-app from this app
    to_multi_app = sub_app

    # The name of the variable in this app
    source_variable = u

    # The name of the auxiliary variable in the sub-app
    variable = tu
  []
[]
