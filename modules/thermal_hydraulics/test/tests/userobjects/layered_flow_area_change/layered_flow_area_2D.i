[Mesh]
  [generated]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 1 1'
    dy = '4'
    ix = '1 1 1'
    iy = '40'
    subdomain_id = '1 2 1'
  []

  [interior_walls]
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = 1
    paired_block = 2
    new_boundary = interior
    input = generated
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [disp_x]
  []

  [disp_y]
  []

  [channel_dA]
  []
[]

[ICs]
  [disp_x_ic]
    type = FunctionIC
    variable = disp_x
    function = 'if (x < 1.5, 0.5 * (y - 2) * 0.1, 0)'
  []
[]

[AuxKernels]
  [channel_dA_aux]
    type = SpatialUserObjectAux
    variable = channel_dA
    user_object = layered_area_change
  []
[]

[UserObjects]
  [layered_area_change]
    type = LayeredFlowAreaChange
    direction = y
    displacements = 'disp_x disp_y'
    boundary = interior
    num_layers = 40
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
