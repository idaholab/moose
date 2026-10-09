[Mesh]
  file = ../transient_heat/cube.e
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [heat]
    type = KokkosHeatConduction
    variable = u
  []

  [ie]
    type = KokkosSpecificHeatConductionTimeDerivative
    variable = u
  []
[]

[BCs]
  [bottom]
    type = KokkosDirichletBC
    variable = u
    boundary = 1
    value = 0.0
  []

  [top]
    type = KokkosDirichletBC
    variable = u
    boundary = 2
    value = 1.0
  []
[]

[Materials]
  [constant]
    type = KokkosHeatConductionMaterial
    block = 1
    thermal_conductivity = 1
    specific_heat = 1
  []
  [density]
    type = KokkosGenericConstantMaterial
    block = 1
    prop_names = density
    prop_values = 1
  []
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
  exodus = true
[]
