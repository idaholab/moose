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
  [vt]
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
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

[Materials]
  [diff]
    type = ParsedMaterial
    property_name = D
    coupled_variables = 'vt'
    expression = 'vt'
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
  [micro]
    type = TransientMultiApp
    positions = '0.15 0.15 0  0.45 0.45 0  0.75 0.75 0'
    input_files = '01_sub.i'
    execute_on = timestep_end
    output_in_position = true
  []
[]

[Transfers]
  [push_u]
    type = MultiAppVariableValueSampleTransfer
    to_multi_app = micro
    source_variable = u
    variable = ut
  []

  [pull_v]
    type = MultiAppPostprocessorInterpolationTransfer
    from_multi_app = micro
    variable = vt
    postprocessor = average_v
  []
[]
