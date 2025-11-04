[Problem]
  kernel_coverage_check = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmax = 1
    ymax = 1
    nx = 25
    ny = 25
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = 'gen'
    block_id = 1
    bottom_left = '0 0 0'
    top_right = '0.3 1 0'
  []
  [middle]
    type = SubdomainBoundingBoxGenerator
    input = 'left'
    block_id = 2
    bottom_left = '0.3 0 0'
    top_right = '0.6 1 0'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = 'middle'
    block_id = 3
    bottom_left = '0.6 0 0'
    top_right = '1 1 0'
  []
[]

[Variables]
  [u]
  []
[]

[ICs]
  [u_1]
    type = ConstantIC
    variable = 'u'
    value = 1
    block = 1
  []
  [u_2]
    type = ConstantIC
    variable = 'u'
    value = 2
    block = 2
  []
  [u_3]
    type = ConstantIC
    variable = 'u'
    value = 3
    block = 3
  []
[]

[MeshModifiers]
  [moving_circle_bottom]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'phi_1'
    criterion_type = 'BELOW'
    threshold = 0
    subdomain_id = 1
    reinitialize_subdomains = '1 2'
    old_subdomain_reinitialized = false
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [moving_circle_top]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'phi_2'
    criterion_type = 'BELOW'
    threshold = 0
    subdomain_id = 2
    reinitialize_subdomains = '1 2'
    old_subdomain_reinitialized = false
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[AuxVariables]
  [phi_1]
  []
  [phi_2]
  []
[]

[AuxKernels]
  [phi_1]
    type = ParsedAux
    variable = 'phi_1'
    expression = '(x-t)^2+(y)^2-0.3^2'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [phi_2]
    type = ParsedAux
    variable = 'phi_2'
    expression = '(x-t)^2+(y-1)^2-0.3^2'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  dt = 0.3
  num_steps = 3
[]

[Outputs]
  exodus = true
[]

