[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Variables/u]
[]

[Executioner]
  type = Steady
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Reporters]
  [initialSetup]
    type = TestDeclareInitialSetupReporter
    value = 1980
  []
  [get]
    type = TestGetReporterDeclaredInInitialSetupReporter
    other_reporter = initialSetup/value
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
  []
[]
