[Mesh]
  file = square.e
  # This MeshModifier currently only works with SerialMesh.
  # For more information, refer to #2129.
  distribution = serial
[]

[MeshModifiers]
  [./middle_node]
    type = AddExtraNodeset
    boundary = 'middle_node'
    nodes = '2'
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right middle'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]

  [./middle]
    type = DirichletBC
    variable = u
    boundary = 'middle_node'
    value = -1
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Output]
  linear_residuals = true
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
