[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
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
  [../]
[]

[Transfers]
  [./to_sub]
    type = MultiAppShapeEvaluationTransfer
    source_variable = u
    variable = aux_u
    to_multi_app = sub
    from_postprocessors_to_be_preserved  = 'from_postprocessor'
    to_postprocessors_to_be_preserved  = 'to_postprocessor'
  [../]
[]

[Outputs]
  exodus = true
  [./console]
    type = Console
    execute_postprocessors_on = 'INITIAL nonlinear TIMESTEP_END'
  [../]
[]
