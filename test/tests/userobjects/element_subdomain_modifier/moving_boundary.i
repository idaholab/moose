[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 1
    ny = 3
    xmax = 1
    ymax = 3
  []
  [block1]
    type = SubdomainBoundingBoxGenerator
    input = gmg
    block_id = 1
    block_name = active
    bottom_left = '0 0 0'
    top_right = '1 1 0'
  []
  [block2]
    type = SubdomainBoundingBoxGenerator
    input = block1
    block_id = 2
    block_name = inactive
    bottom_left = '0 1 0'
    top_right = '1 3 0'
  []
  [moving_boundary]
    type = SideSetsAroundSubdomainGenerator
    input = block2
    block = 1
    new_boundary = moving
  []
[]

[Variables]
  [temperature]
    initial_condition = 298
  []
[]

[AuxVariables]
  [u]
    [AuxKernel]
      type = FunctionAux
      function = 't-y'
      execute_on = 'INITIAL TIMESTEP_BEGIN'
    []
  []
[]

[Kernels]
  [Tdot]
    type = TimeDerivative
    variable = temperature
  []
  [heat_conduction]
    type = Diffusion
    variable = temperature
  []
[]

[UserObjects]
  [w_mvg_bnd]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'u'
    block = '1 2'
    criterion_type = ABOVE
    threshold = 0
    subdomain_id = 1
    active_subdomains = 1
    moving_boundary_name = 'moving'
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  end_time = 5
  dt = 1
  solve_type = NEWTON
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  line_search = none
  nl_abs_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
