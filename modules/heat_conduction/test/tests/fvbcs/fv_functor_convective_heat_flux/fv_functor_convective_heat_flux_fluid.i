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

  [remove]
    type = BlockDeletionGenerator
    input = gen
    block = 0
    new_boundary = interface
  []
[]

[Variables]
  [T_fluid]
    type = MooseVariableFVReal
    initial_condition = 0
  []
[]

[AuxVariables]
  [T_solid]
    type = MooseVariableFVReal
    initial_condition = 0
  []
[]

[FVKernels]
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
  [interface_fluid]
    type = FVFunctorConvectiveHeatFluxBC
    boundary = 'interface'
    variable = T_fluid
    T_bulk = T_fluid
    T_solid = T_solid
    is_solid = false
    heat_transfer_coefficient = 'htc'
  []
  [left]
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
