[Mesh]
  type = GeneratedMesh
  dim = 3

  nx = 10
  ny = 10
  nz = 10

  zmax = 3
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [v_average]
    order = CONSTANT
    family = MONOMIAL
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
  [front]
    type = DirichletBC
    variable = u
    boundary = front
    value = 0
  []
  [back]
    type = DirichletBC
    variable = u
    boundary = back
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

[UserObjects]
  [layered_integral]
    type = NearestPointLayeredIntegral
    points = '0.15 0.15 0  0.45 0.45 0  0.75 0.75 0'
    direction = z
    num_layers = 4
    variable = u
  []
[]

[MultiApps]
  [sub_app]
    type = TransientMultiApp
    positions = '0.15 0.15 0  0.45 0.45 0  0.75 0.75 0'
    input_files = '03_sub_uot.i'
    execute_on = timestep_end
    output_in_position = true
  []
[]

[Transfers]
  [push_u]
    type = MultiAppUserObjectTransfer
    to_multi_app = sub_app
    variable = u_integral
    user_object = layered_integral
  []

  [pull_v]
    type = MultiAppUserObjectTransfer
    from_multi_app = sub_app
    variable = v_average
    user_object = layered_average
  []
[]
