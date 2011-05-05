[Mesh]
  file = rectangle.e
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

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Postprocessors]
  [./integral_left]
    type = ElementIntegral
    variable = u
    block = 1
  [../]
  
  [./integral_right]
    type = ElementIntegral
    variable = u
    block = 2
  [../]

  [./integral_all]
    type = ElementIntegral
    variable = u
  [../]
[]

[Output]
  file_base = out_block
  output_initial = true
  interval = 1
  exodus = false
  postprocessor_csv = true
  perf_log = true
[]
