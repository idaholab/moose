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
  active = 'convected'

  [./convected]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff conv'

  [./diff]
    type = DiffusionPrecompute
    variable = convected
  [../]

  [./conv]
    type = ConvectionPrecompute
    variable = convected
    velocity = '1.0 0.0 0.0'
  [../]
[]

[BCs]
  active = 'bottom top'

  [./bottom]
    type = DirichletBC
    variable = convected
    boundary = 'left'
    value = 0
  [../]

  [./top]
    type = DirichletBC
    variable = convected
    boundary = 'right'
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  perf_log = true
[]


