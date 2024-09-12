[Problem]
  solve = false
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 16
    ny = 16
  []
  [left]
    type = SubdomainBoundingBoxGenerator
    input = 'gen'
    block_id = 1
    block_name = 'left'
    bottom_left = '0 0 0'
    top_right = '0.25 1 1'
  []
  [right]
    type = SubdomainBoundingBoxGenerator
    input = 'left'
    block_id = 2
    block_name = 'right'
    bottom_left = '0.25 0 0'
    top_right = '1 1 1'
  []
[]

[AuxVariables]
  [u]
  []
  [phi]
  []

  # for the 'displaced' test only
  inactive = 'disp_x disp_y'
  [disp_x]
  []
  [disp_y]
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
    value = -0.5
    block = 2
  []
[]

[MeshModifiers]
  [moving_circle]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = 'phi'
    criterion_type = 'BELOW'
    threshold = 0
    subdomain_id = 1
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[AuxKernels]
  [phi]
    type = ParsedAux
    variable = 'phi'
    expression = '(x-t)^2+(y)^2-0.5^2'
    use_xyzt = true
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
  [double_u]
    type = StatefulAux
    variable = 'u'
    coupled = 'u'
    block = 1
  []
[]

[Postprocessors]
  # for the 'subdomain_caching' test only
  active = ''
  [average]
    type = SideAverageValue
    variable = 'u'
    boundary = 'bottom'
    execute_on = 'INITIAL TIMESTEP_BEGIN'
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 3
[]

[Outputs]
  [out]
    type = Exodus
  []
[]
