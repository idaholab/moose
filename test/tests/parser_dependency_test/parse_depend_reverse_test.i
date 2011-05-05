[Output]
  file_base = 2d_diffusion_out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
   
[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 1
  [../]
[]

[BCs]
  active = 'left right'

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
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Mesh]
  dim = 2
  file = square.e
[]


