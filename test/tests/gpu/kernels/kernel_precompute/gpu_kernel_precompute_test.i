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

[GPUKernels]
  active = 'diff conv'

  [./diff]
    type = GPUDiffusionPrecompute
    variable = convected
  [../]

  [./conv]
    type = GPUConvectionPrecompute
    variable = convected
    velocity = '1.0 0.0 0.0'
  [../]
[]

[GPUBCs]
  active = 'bottom top'

  [./bottom]
    type = GPUDirichletBC
    variable = convected
    boundary = 'left'
    value = 0
  [../]

  [./top]
    type = GPUDirichletBC
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
  file_base = out_gpu
  exodus = true
[]
