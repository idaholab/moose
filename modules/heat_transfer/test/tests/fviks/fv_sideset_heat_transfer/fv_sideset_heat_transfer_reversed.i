[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 5
    xmax = 2
    subdomain_ids = '0 0 0 1 1 1
                     0 0 0 1 1 1
                     0 0 0 1 1 1
                     0 0 0 1 1 1
                     0 0 0 1 1 1'
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = gen
    primary_block = 0
    paired_block = 1
    new_boundary = interface
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [T_left]
    type = MooseVariableFVReal
    initial_condition = 1
    block = 0
  []

  [T_right]
    type = MooseVariableFVReal
    initial_condition = 0
    block = 1
  []
[]

[FVKernels]
  [diff_left]
    type = FVDiffusion
    variable = T_left
    block = 0
    coeff = 2
  []

  [diff_right]
    type = FVDiffusion
    variable = T_right
    block = 1
    coeff = 4
  []
[]

[FVInterfaceKernels]
  [interface_heat_transfer]
    type = FVSideSetHeatTransferKernel
    boundary = interface
    subdomain1 = 1
    subdomain2 = 0
    variable1 = T_right
    variable2 = T_left
    conductance = interface_conductance
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    boundary = left
    variable = T_left
    value = 1
  []

  [right]
    type = FVDirichletBC
    boundary = right
    variable = T_right
    value = 0
  []
[]

[FunctorMaterials]
  [interface_conductance]
    type = ADGenericFunctorMaterial
    prop_names = 'interface_conductance'
    prop_values = '1'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
