[Mesh]
  file = cube.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = u
  [../]

  [./ie]
    type = HeatConductionTimeDerivative
    variable = u
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0.0
  [../]

  [./top]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1.0
  [../]
[]

[Materials]
  [./constant]
    type = HeatConductionMaterial
    block = 1
    thermal_conductivity = 1
    specific_heat = 1
  [../]
  [./density]
    type = Density
    block = 1
    density = 1
  [../]
[]

[Executioner]
  type = Transient
  petsc_options = '-snes_mf_operator'

  start_time = 0.0
  num_steps = 5
  dt = .1
[]

[Output]
  file_base = out
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]

