[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmax = 100
  ymax = 100
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
    hold_time = 2
    file = nuclei2.csv
  [../]
  [./map]
    type = DiscreteNucleationMap
    int_width = 3
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
