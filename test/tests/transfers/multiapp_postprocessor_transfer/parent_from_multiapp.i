[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[AuxVariables]
  [./from_sub]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Postprocessors]
  [./sub_average]
    type = Receiver
  [../]
  [./sub_sum]
    type = Receiver
  [../]
  [./sub_maximum]
    type = Receiver
  [../]
  [./sub_minimum]
    type = Receiver
  [../]
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
  [./sub]
    positions = '0.2 0.2 0 0.7 0.7 0'
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = 'sub0.i sub1.i'
  [../]
[]

[Transfers]
  [./pp_transfer_ave]
    type = MultiAppPostprocessorTransfer
    reduction_type = average
    from_multi_app = sub
    from_postprocessor = average
    to_postprocessor = sub_average
  [../]
  [./pp_transfer_sum]
    type = MultiAppPostprocessorTransfer
    reduction_type = sum
    from_multi_app = sub
    from_postprocessor = average
    to_postprocessor = sub_sum
  [../]
  [./pp_transfer_min]
    type = MultiAppPostprocessorTransfer
    reduction_type = minimum
    from_multi_app = sub
    from_postprocessor = average
    to_postprocessor = sub_minimum
  [../]
  [./pp_transfer_max]
    type = MultiAppPostprocessorTransfer
    reduction_type = maximum
    from_multi_app = sub
    from_postprocessor = average
    to_postprocessor = sub_maximum
  [../]
[]
