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
  [./subsub_average]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./force]
    type = CoupledForce
    variable = u
    v = subsub_average
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
    type = ElementAverageValue
    variable = u
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
  dt = 0.3

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '0 0 0 0.5 0.5 0'
    input_files = multilevel_subsub.i
  [../]
[]

[Transfers]
  [./subsub_average]
    type = MultiAppPostprocessorInterpolationTransfer
    from_multi_app = sub
    variable = subsub_average
    postprocessor = subsub_average
  [../]
[]
