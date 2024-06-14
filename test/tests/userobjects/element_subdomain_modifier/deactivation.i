[Problem]
  kernel_coverage_check = false
[]

[Mesh]
    add_subdomain_ids = 100
    [gmg]
      type = GeneratedMeshGenerator
      dim = 2
      nx = 3
      ny = 2
      xmax = 3
      ymax = 2
    []
    [base]
      type = SubdomainBoundingBoxGenerator
      input = 'gmg'
      block_id = 1
      block_name = 'base'
      bottom_left = '0 0 0'
      top_right = '3 1 0'
    []
    [block_1]
      type = SubdomainBoundingBoxGenerator
      input = 'base'
      block_id = 2
      block_name = 'block_1'
      bottom_left = '0 1 0'
      top_right = '1 2 0'
    []
    [block_2]
      type = SubdomainBoundingBoxGenerator
      input = 'block_1'
      block_id = 3
      block_name = 'block_2'
      bottom_left = '1 1 0'
      top_right = '2 2 0'
    []
    [block_3]
      type = SubdomainBoundingBoxGenerator
      input = 'block_2'
      block_id = 4
      block_name = 'block_3'
      bottom_left = '2 1 0'
      top_right = '3 2 0'
    []
    [moving_boundary]
      type = SideSetsAroundSubdomainGenerator
      input = 'block_3'
      block = 1
      new_boundary = 'moving'
    []
  []
  
  [Variables]
    [temperature]
      initial_condition = 298
      block = 'base'
    []
  []
  
  [AuxVariables]
    [u]
      family = MONOMIAL
      order = CONSTANT
    []
  []
  
  [Kernels]
    [Tdot]
      type = TimeDerivative
      variable = 'temperature'
      block = 'base'
    []
    [heat_conduction]
      type = Diffusion
      variable = 'temperature'
      block = 'base'
    []
  []

  [AuxKernels]
    [block_1]
      type = ParsedAux
      variable = 'u'
      expression = 'if (t>=1,1,0)'
      use_xyzt = true
      block = 'block_1'
    []
    [block_2]
      type = ParsedAux
      variable = 'u'
      expression = 'if (t>=2,1,0)'
      use_xyzt = true
      block = 'block_2'
    []
    [deact]
      type = ParsedAux
      variable = 'u'
      expression = 'if (t>=4 & y < 1 & x < 1,0,1)'
      use_xyzt = true
      block = 'base'
    []
  []
  
  [UserObjects]
    [w_mvg_bnd]
      type = CoupledVarThresholdElementSubdomainModifier
      coupled_var = 'u'
      criterion_type = 'ABOVE'
      threshold = 0.5
      subdomain_id = 1
    #   active_subdomains = 1
      moving_boundaries = 'moving moving moving moving'
      moving_boundary_subdomain_pairs = '1 3;1 4; 1 5;1'
      block = 'block_1 block_2'
    []
    [deactivation]
      type = CoupledVarThresholdElementSubdomainModifier
      coupled_var = 'u'
      criterion_type = 'BELOW'
      threshold = 0.5
      subdomain_id = 100
    #   active_subdomains = 1
      moving_boundaries = 'moving'
      moving_boundary_subdomain_pairs = '1 100'
      block = 'base'
    []
  []
  
  [Executioner]
    type = Transient
    end_time = 5
    dt = 1
    solve_type = 'NEWTON'
    petsc_options_iname = '-pc_type'
    petsc_options_value = 'lu'
    line_search = 'none'
    nl_abs_tol = 1e-10
  []
  
  [Outputs]
    exodus = true
  []