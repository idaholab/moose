[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 2
  ny = 2
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right_dirichlet]
    type = OnOffDirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
  [./right_neumann]
    type = OnOffNeumannBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Transient

  start_time = 0
  end_time = 1
  dt = 0.1


  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
  exodus = true
[]
