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
  [./convected]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = ADDiffusion
    variable = convected
  [../]

  [./conv]
    type = ADConvectionPrecompute
    variable = convected
    velocity = '1.0 0.0 0.0'
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = convected
    preset = false
    boundary = 'left'
    value = 0
  [../]

  [./top]
    type = DirichletBC
    variable = convected
    preset = false
    boundary = 'right'
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out
  exodus = true
[]
