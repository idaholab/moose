[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  nz = 0
  xmin = 0
  xmax = 20
  ymin = 0
  ymax = 20
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[UserObjects]
  [./inserter]
    type = DiscreteNucleationInserter
    hold_time = 0.5
    probability = 0.0076
    radius = 3.27
  [../]
[]

[Postprocessors]
  [./nuc_count]
    type = DiscreteNucleationData
    inserter = inserter
    value = COUNT
  [../]
  [./nuc_update]
    type = DiscreteNucleationData
    inserter = inserter
    value = UPDATE
  [../]
  [./nuc_rate]
    type = DiscreteNucleationData
    inserter = inserter
    value = RATE
  [../]
  [./nuc_insertions]
    type = DiscreteNucleationData
    inserter = inserter
    value = INSERTIONS
  [../]
  [./nuc_deletions]
    type = DiscreteNucleationData
    inserter = inserter
    value = DELETIONS
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.55
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
[]
