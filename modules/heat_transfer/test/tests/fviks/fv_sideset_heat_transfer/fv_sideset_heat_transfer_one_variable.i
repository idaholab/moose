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
  [T]
    type = MooseVariableFVReal
    initial_condition = 0.5
  []
[]

[FVKernels]
  [diff_left]
    type = FVDiffusion
    variable = T
    block = 0
    coeff = 2
  []

  [diff_right]
    type = FVDiffusion
    variable = T
    block = 1
    coeff = 4
  []
[]

[FVInterfaceKernels]
  [interface_heat_transfer]
    type = FVSideSetHeatTransferKernel
    boundary = interface
    subdomain1 = 0
    subdomain2 = 1
    variable1 = T
    conductance = constant_conductance
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    boundary = left
    variable = T
    value = 1
  []

  [right]
    type = FVDirichletBC
    boundary = right
    variable = T
    value = 0
  []
[]

[FunctorMaterials]
  [constant_conductance]
    type = ADGenericFunctorMaterial
    prop_names = 'constant_conductance'
    prop_values = '1'
  []
  [piecewise_conductance]
    type = ADPiecewiseByBlockFunctorMaterial
    prop_name = 'piecewise_conductance'
    subdomain_to_prop_value = '0 1
                               1 100'
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
