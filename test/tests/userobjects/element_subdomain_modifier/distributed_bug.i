[Problem]
  kernel_coverage_check = false
[]
  
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1000
    ny = 100
    xmax = 20
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
    top_right = '20 1 0'
  []
  [moving_boundary]
    type = SideSetsAroundSubdomainGenerator
    input = 'block_2'
    block = 1
    new_boundary = 'moving'
  []
[]
    
[Variables]
  [temperature]
    initial_condition = 298
    block = 'block_1'
  []
[]
    
[AuxVariables]
  [u]
    [AuxKernel]
      type = FunctionAux
      function = 't-x'
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
    moving_boundaries = 'moving moving'
    moving_boundary_subdomain_pairs = '1 2;1'
  []
[]
    
[Executioner]
  type = Transient
  end_time = 20
  dt = 1
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = 'none'
  nl_abs_tol = 1e-10
[]

[Outputs]
  [perf_graph]
    type = PerfGraphOutput
    level = 4
  []
[]