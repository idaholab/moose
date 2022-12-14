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
  active = initialSetup
  [initialSetup]
    type = TestDeclareInitialSetupReporter
    value = 1980
  []
  [info]
    type = MeshInfo
    items = num_elements
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
  []
[]
