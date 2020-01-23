[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 'left'
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    preset = false
    boundary = 'right'
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  mffd_type = 'ds'
[]
