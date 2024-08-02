[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1
    ymin = -1

    nx = 2
    ny = 2
    subdomain_ids = '0 0 1 1'
  []
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[AuxVariables]
  [c]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [c]
    type = FunctionAux
    variable = c
    function = 'if(t%2,x>0,x<0)'
  []
[]

[MeshModifiers]
  [subdomain_updater]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = c
    threshold = 0.5
    subdomain_id = 1
    complement_subdomain_id = 0
    execute_on = TIMESTEP_END
    execution_order_group = 0
  []
  [side_updater]
    type = SidesetAroundSubdomainUpdater
    inner_subdomains = 0
    outer_subdomains = 1
    update_sideset_name = top
    execute_on = TIMESTEP_END
    execution_order_group = 1
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  exodus = true
[]
