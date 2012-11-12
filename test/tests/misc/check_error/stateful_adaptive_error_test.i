[Mesh]
  file = square.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ie]
    type = ImplicitEuler
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

[Materials]
  active = empty

  # Stateful Materials are on
  [./empty]
    type = StatefulMaterial
    block = 1
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  petsc_options = '-snes_mf_operator'

  # Adaptivity is on
  [./Adaptivity]
  [../]
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]


