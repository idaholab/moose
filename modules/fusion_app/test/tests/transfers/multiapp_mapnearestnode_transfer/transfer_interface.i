[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 40
    xmax = 2
    ny = 40
    ymax = 2
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.1 0.1 0'
    top_right = '0.6 0.6 0'
    block_id = 1
  []
  [subdomain2]
    input = subdomain1
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1. 1. 0'
    top_right = '1.5 1.5 0'
    block_id = 2
  []

  [interface1]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain2
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'boundary_interface_1'
  []

  [interface2]
    type = SideSetsBetweenSubdomainsGenerator
    input = interface1
    primary_block = '0'
    paired_block = '2'
    new_boundary = 'boundary_interface_2'
  []

  [subdomain3]
    input = interface2
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1.6 0.5 0'
    top_right = '1.9 1.5 0'
    block_id = 3
  []

  [interface3]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain3
    primary_block = '0'
    paired_block = '3'
    new_boundary = 'boundary_interface_3'
  []

  [ed0]
    type = BlockDeletionGenerator
    input = interface3
    block = "1 2 3"
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff_u]
    type = MatDiffusion
    variable = u
    diffusivity = D
    block = 0
  []
  [source_u]
    type = BodyForce
    variable = u
    value = 1
    block = 0
  []
[]

[BCs]
  [u]
    type = VacuumBC
    variable = u
    boundary = 'left bottom right top'
  []

  [interface1]
    type = DirichletBC
    variable = u
    value = 10
    boundary = boundary_interface_1
  []

  [interface2]
    type = DirichletBC
    variable = u
    value = 100
    boundary = boundary_interface_2
  []

  [interface3]
    type = DirichletBC
    variable = u
    value = 50
    boundary = boundary_interface_3
  []
[]

[AuxVariables]
  [from_sub]
  []
  [from_sub_elemental_var]
  []
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]

[Materials]
  [mat0]
    type = GenericConstantMaterial
    prop_names = 'D'
    prop_values = '1'
    block = 0
  []
[]

[MultiApps]
  [channel]
    type = FullSolveMultiApp
    input_files = 'transfer_power.i'
    positions = '1.05 1.05 0 1.05 1.05 0 1.05 1.05 0'
    cli_args = 'Problem/master_bdry_name=boundary_interface_1;BCs/left/value=5;BCs/right/value=10
     Problem/master_bdry_name=boundary_interface_2;BCs/left/value=20;BCs/right/value=50
     Problem/master_bdry_name=boundary_interface_3;BCs/left/value=60;BCs/right/value=100'
    max_procs_per_app = 1
    keep_solution_during_restore = true
    output_in_position = true
    execute_on = 'timestep_end'
  []
[]

[Transfers]
#  active = 'from_subapp_elememtal'

  [to_subapp]
    type = MultiAppMapNearestNodeTransfer
    to_multi_app = channel
    source_variable = u
    variable = from_master
  []

  [to_subapp_elemental]
    type = MultiAppMapNearestNodeTransfer
    to_multi_app = channel
    source_variable = u
    variable = from_master_elemental
  []

  [from_subapp]
    type = MultiAppMapNearestNodeTransfer
    from_multi_app = channel
    source_variable = temp
    variable = from_sub
  []

  [from_subapp_elememtal]
    type = MultiAppMapNearestNodeTransfer
    from_multi_app = channel
    source_variable = to_master
    variable = from_sub_elemental_var
  []
[]
