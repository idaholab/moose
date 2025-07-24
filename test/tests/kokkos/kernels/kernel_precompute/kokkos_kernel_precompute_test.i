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

[KokkosKernels]
  active = 'diff conv'

  [./diff]
    type = KokkosDiffusionPrecompute
    variable = convected
  [../]

  [./conv]
    type = KokkosConvectionPrecompute
    variable = convected
    velocity = '1.0 0.0 0.0'
  [../]
[]

[KokkosBCs]
  active = 'bottom top'

  [./bottom]
    type = KokkosDirichletBC
    variable = convected
    boundary = 'left'
    value = 0
  [../]

  [./top]
    type = KokkosDirichletBC
    variable = convected
    boundary = 'right'
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
