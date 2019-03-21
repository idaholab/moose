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

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[MultiApps]
  [./sub]
    type = FullSolveMultiApp
    input_files = sub_conservative_transfer.i
    execute_on = timestep_end
  [../]
[]

[Postprocessors]
  [./from_postprocessor]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = 'Nonlinear TIMESTEP_END'
  [../]
[]

[Transfers]
  [./to_sub]
    type = MultiAppNearestNodeTransfer
    direction = to_multiapp
    source_variable = u
    variable = aux_u
    preserve_transfer = true
    multi_app = sub
  [../]
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    execute_postprocessors_on = 'INITIAL nonlinear TIMESTEP_END'
  [../]
[]
