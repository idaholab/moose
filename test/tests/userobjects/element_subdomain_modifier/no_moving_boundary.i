[Problem]
  solve = false
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 1
    xmax = 3
    ymax = 1
  []
  [block_1]
    type = SubdomainBoundingBoxGenerator
    input = 'gmg'
    block_id = 1
    block_name = 'block_1'
    bottom_left = '0 0 0'
    top_right = '1 1 0'
  []
  [block_2]
    type = SubdomainBoundingBoxGenerator
    input = 'block_1'
    block_id = 2
    block_name = 'block_2'
    bottom_left = '1 0 0'
    top_right = '2 1 0'
  []
  [block_3]
    type = SubdomainBoundingBoxGenerator
    input = 'block_2'
    block_id = 3
    block_name = 'block_3'
    bottom_left = '2 0 0'
    top_right = '3 1 0'
  []
[]

[AuxVariables]
  [u]
    [AuxKernel]
      type = FunctionAux
      function = 't-x'
      execute_on = 'INITIAL TIMESTEP_BEGIN'
    []
  []
[]

[UserObjects]
  [w_mvg_bnd]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'u'
    criterion_type = 'ABOVE'
    threshold = 0
    subdomain_id = 1
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  end_time = 3
  dt = 1
[]

[Outputs]
  exodus = true
[]
