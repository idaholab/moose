[Mesh]
  [generated]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
    xmax = 2
    ymax = 1
  []
[]

[Variables]
  [T]
    initial_condition = 3750
  []
[]

[BCs]
  [t_left]
    type = DirichletBC
    variable = T
    value = 300
    boundary = 'left'
  []
  [t_right]
    type = DirichletBC
    variable = T
    value = 3750
    boundary = 'right'
  []
[]

[Kernels]
  [heat_conduction]
    type = HeatConduction
    variable = T
  []
[]

[Materials]
  [thermal]
    type = TungstenThermalPropertiesMaterial
    temperature = T
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  type = FEProblem
  allow_invalid_solution = false
  immediately_print_invalid_solution = false
[]

[Reporters]
  [solution_invalidity]
    type = SolutionInvalidityReporter
    execute_on = FINAL
  []
[]

[Outputs]
  file_base = 'invalid_temperature'
  [out]
    type = JSON
    execute_on = 'FINAL'
    execute_system_information_on = none
  []
[]

