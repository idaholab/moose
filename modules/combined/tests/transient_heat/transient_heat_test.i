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
    type = SpecificHeatConductionTimeDerivative
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  start_time = 0.0
  num_steps = 5
  dt = .1
[]

[Outputs]
  file_base = out
  exodus = true
[]
