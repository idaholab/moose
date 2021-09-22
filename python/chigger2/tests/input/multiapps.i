[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 0.1
  solve_type = PJFNK
[]

[MultiApps]
  [./sub]
    app_type = MooseTestApp
    type = TransientMultiApp
    input_files = sub.i
    output_in_position = true
    positions = '0 0 0
                 0 0 0.25
                 0 0 0.5
                 0 0 0.75
                 0 0 1'
  [../]
[]
