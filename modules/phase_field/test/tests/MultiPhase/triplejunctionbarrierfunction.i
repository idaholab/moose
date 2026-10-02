[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  xmin = 0
  xmax = 1
[]

[AuxVariables]
  [./eta1]
    initial_condition = 0.5
  [../]
  [./eta2]
    initial_condition = 0.6
  [../]
  [./eta3]
    initial_condition = 0.7
  [../]
  [./eta4]
    initial_condition = 0.8
  [../]
[]

[Materials]
  [./obstacle_uniform]
    type = TripleJunctionBarrierFunctionMaterial
    etas = 'eta1 eta2 eta3 eta4'
    function_name = f_obs_uniform
    h = 2.5
    outputs = exodus
  [../]
  [./obstacle_per_triple]
    type = TripleJunctionBarrierFunctionMaterial
    etas = 'eta1 eta2 eta3 eta4'
    function_name = f_obs_triples
    h_ijk = '1.0 2.0 3.0 4.0'
    outputs = exodus
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
[]
