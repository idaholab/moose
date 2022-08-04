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
    block = 1
    new_boundary = interface
  []
[]

[Variables]
  [T_solid]
    type = MooseVariableFVReal
    initial_condition = 1
  []
[]

[AuxVariables]
  [T_fluid]
    type = MooseVariableFVReal
    initial_condition = 0
  []
[]

[FVKernels]
  [diff_wall]
    type = FVDiffusion
    variable = T_solid
    block = 0
    coeff = 2
  []
[]

[FVBCs]
  [interface_fluid]
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
[]

[MultiApps]
  [interface_multiapp]
    type = FullSolveMultiApp
    input_files = 'fv_functor_convective_heat_flux_fluid.i'
    execute_on = TIMESTEP_END
  []
[]

[Transfers]
  [solid_transfer]
    type = MultiAppNearestNodeTransfer
    source_variable = T_solid
    variable = T_solid
    to_multi_app = interface_multiapp
  []
  [fluid_transfer]
    type = MultiAppNearestNodeTransfer
    source_variable = T_fluid
    variable = T_fluid
    from_multi_app = interface_multiapp
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
