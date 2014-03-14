[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 0
  zmin = 0
  zmax = 0
  elem_type = QUAD4
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
  active = 'left right'

  # We will use PresetBCs
  [./left]
    type = PresetBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = PresetBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  linear_residuals = true
  file_base = bc_preset_out
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
