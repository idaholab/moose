[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 10
  ny = 10
  nz = 15

  xmin = 0
  xmax = 2

  ymin = 0
  ymax = 2

  zmin = 0
  zmax = 1
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

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]

[Postprocessors]
  [./average]
    type = CriticalTimeStep
    poisson_ratio = 0.3
    youngs_modulus = 200000
    material_density = 8050
  [../]
[]

[Outputs]
  csv = true
  file_base = out
  []
