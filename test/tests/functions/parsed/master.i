[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[AuxVariables]
  [f]
  []
[]

[Problem]
  solve = false
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    input_files = 'sub.i'
    positions = '1.0 2.0 3.0'
    output_in_position = true
  []
[]

[Executioner]
  type = Steady
[]
