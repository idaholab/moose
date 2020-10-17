[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 10
  ymax = 10
[]

[Variables]
  [./dummy]
  []
[]

[AuxVariables]
  [./c]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./c]
    type = DiscreteNucleationAux
    variable = c
    map = map
  [../]
[]

[UserObjects]
  [./inserter]
    type = DiscreteNucleationFromFile
    hold_time = 1
    file = nuclei.csv
    radius = 2
  [../]
  [./map]
    type = DiscreteNucleationMap
    int_width = 1
    inserter = inserter
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  num_steps = 10
  dt = 0.5
[]

[Problem]
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
  hide = dummy
[]
