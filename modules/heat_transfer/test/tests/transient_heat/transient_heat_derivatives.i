[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

[Variables]
  [temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 2
  []
[]

[Kernels]
  [heat]
    type = HeatConduction
    variable = temp
  []

  [ie]
    type = HeatConductionTimeDerivative
    variable = temp
    specific_heat_dT = specific_heat_dT
    density_name_dT = density_dT
  []
[]

[Functions]
  [spheat]
    type = ParsedFunction
    expression = 't^4'
  []
  [thcond]
    type = ParsedFunction
    expression = 'exp(t)'
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = temp
    boundary = 1
    value = 4
  []

  [top]
    type = DirichletBC
    variable = temp
    boundary = 2
    value = 1
  []
[]

[Materials]
  [constant]
    type = HeatConductionMaterial
    thermal_conductivity_temperature_function = thcond
    specific_heat_temperature_function = spheat
    temp = temp
  []
  [density]
    type = ParsedMaterial
    property_name = density
    coupled_variables = temp
    expression = 'temp^3 + 2/temp'
  []
  [density_dT]
    type = ParsedMaterial
    property_name = density_dT
    coupled_variables = temp
    expression = '3 * temp^2 - 2/temp/temp'
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1
  dt = .1
  nl_max_its = 10
  dtmin = .1
[]

[Postprocessors]
  [avg]
    type = ElementAverageValue
    variable = temp
  []
[]

[Outputs]
  csv = true
[]
