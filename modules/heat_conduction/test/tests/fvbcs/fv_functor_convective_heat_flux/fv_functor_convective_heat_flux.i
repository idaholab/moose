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
  [T_solid]
    type = MooseVariableFVReal
    initial_condition = 1
    block = 0
  []

  [T_fluid]
    type = MooseVariableFVReal
    initial_condition = 0
    block = 1
  []
[]

[FVKernels]
  [diff_wall]
    type = FVDiffusion
    variable = T_solid
    block = 0
    coeff = 2
  []
  [diff_fluid]
    type = FVDiffusion
    variable = T_fluid
    block = 1
    coeff = 4
  []
  [gradient_creating]
    type = FVBodyForce
    variable = T_fluid
  []
[]

[FVBCs]
  [interface_fluid_to_solid]
    type = FVFunctorConvectiveHeatFluxBC
    boundary = 'interface'
    variable = T_solid
    T_bulk = T_fluid
    T_solid = T_solid
    is_solid = true
    heat_transfer_coefficient = 'htc'
  []
  [left]
    type = FVDirichletBC
    boundary = 'left'
    variable = T_solid
    value = 1
  []

  [interface_solid_to_fluid]
    type = FVFunctorConvectiveHeatFluxBC
    boundary = 'interface'
    variable = T_fluid
    T_bulk = T_fluid
    T_solid = T_solid
    is_solid = false
    heat_transfer_coefficient = 'htc'
  []
  [right]
    type = FVDirichletBC
    boundary = 'right'
    variable = T_fluid
    value = 0
  []
[]

[Materials]
  [cht]
    type = ADGenericFunctorMaterial
    prop_names = 'htc'
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
