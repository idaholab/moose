[Mesh]
  dim = 2

  [./Generation]
    nx = 10
    ny = 10
    
    xmin = 0
    xmax = 2

    ymin = 0
    ymax = 2
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

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 0
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true
  petsc_options = '-snes_mf_operator'
[]

[Postprocessors]
  [./average]
    type = ElementAverageValue
    variable = u
  [../]
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
[]
   
    
