[Mesh]
  file = mug.e
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
    type = Diffusion
    variable = convected
  [../]

  [./conv]
    type = Convection
    variable = convected
    x = 0.0
    y = 0.0
    z = 1.0
  [../]
[]

[BCs]
  active = 'bottom top'

  [./bottom]
    type = DirichletBC
    variable = convected
    boundary = '1'
    value = 1
  [../]

  [./top]
    type = DirichletBC
    variable = convected
    boundary = '2'
    value = 0
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out
  interval = 1
  exodus = true
  perf_log = true
[]


