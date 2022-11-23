[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [u]
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
  [var]
    family = MONOMIAL
    order = THIRD
  []
[]

[ICs]
  [var_ic]
    type = FunctionIC
    variable = var
    function = '-exp(x * y)'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = secondary_negative_adjuster.i
    execute_on = timestep_begin
  []
[]

[Postprocessors]
  [from_postprocessor]
    type = ElementIntegralVariablePostprocessor
    variable = var
  []
[]

[Transfers]
  [to_sub]
    type = MultiAppShapeEvaluationTransfer
    source_variable = var
    variable = var
    to_multi_app = sub
    from_postprocessors_to_be_preserved  = 'from_postprocessor'
    to_postprocessors_to_be_preserved  = 'to_postprocessor'
    allow_skipped_adjustment = true
  []
[]

[Outputs]
  exodus = true
[]
