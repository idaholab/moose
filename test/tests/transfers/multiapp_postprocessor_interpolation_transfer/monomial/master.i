[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 2
[]

[AuxVariables/from_sub]
  order = CONSTANT
  family = MONOMIAL
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[MultiApps/sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    positions = '0.5 0.25 0 0.5 0.75 0'
    input_files = 'sub.i'
    cli_args = 'Postprocessors/value/function=1949 Postprocessors/value/function=1980'
[]

[Transfers/pp_transfer]
    type = MultiAppPostprocessorInterpolationTransfer
    direction = from_multiapp
    postprocessor = value
    variable = from_sub
    multi_app = sub
[]

[Outputs]
  exodus = true
  execute_on = TIMESTEP_END
[]
