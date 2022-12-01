# This test solves two identical heat conduction problems, one created with THM
# components, and one with the constituent lower-level objects. This test
# illustrates the ability of THM's mesh to be combined with mesh generators
# external to the components system.

rho = 8000
cp = 500
k = 15

initial_T = 1000
T_left = 1005
T_right = 300
htc_right = 1000

[Mesh]
  [gen_mesh_mg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 2
    xmin = 0
    xmax = 5.0
    ymin = 2.0
    ymax = 3.0
  []
  [renumber_block_mg]
    type = RenameBlockGenerator
    input = gen_mesh_mg
    old_block = 0
    new_block = 10
  []
  [renumber_boundary_mg]
    type = RenameBoundaryGenerator
    input = renumber_block_mg
    old_boundary = '0 1 2 3'
    new_boundary = '10 11 12 13'
  []
  # Note that currently, CombinerGenerator only preserves block and boundary
  # names of the first mesh, so any block and boundary naming of the second
  # mesh should occur after the CombinerGenerator; see #22838.
  [merge_mg]
    type = CombinerGenerator
    inputs = 'thm_mesh_generator renumber_boundary_mg'
  []
  [rename_block_mg]
    type = RenameBlockGenerator
    input = merge_mg
    old_block = 10
    new_block = 'hs_moose'
  []
  [rename_boundary_mg]
    type = RenameBoundaryGenerator
    input = rename_block_mg
    old_boundary = '10 11 12 13'
    new_boundary = 'bottom right top left'
  []
[]

[Variables]
  [T_moose]
    block = 'hs_moose'
    initial_condition = ${initial_T}
  []
[]

[Kernels]
  [time_derivative]
    type = ADHeatConductionTimeDerivative
    variable = T_moose
    block = 'hs_moose'
    density_name = density
    specific_heat = specific_heat
  []
  [heat_conduction]
    type = ADHeatConduction
    variable = T_moose
    block = 'hs_moose'
    thermal_conductivity = thermal_conductivity
  []
[]

[BCs]
  [dirichlet_bc]
    type = ADFunctionDirichletBC
    variable = T_moose
    boundary = left
    function = ${T_left}
  []
  [convection_bc]
    type = ADConvectionHeatTransferBC
    variable = T_moose
    boundary = right
    T_ambient = ${T_right}
    htc_ambient = ${htc_right}
  []
[]

[Materials]
  [prop_mat]
    type = ADGenericConstantMaterial
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '${rho} ${cp} ${k}'
  []
[]

[Components]
  [hs]
    type = HeatStructurePlate
    position = '0 0 0'
    orientation = '1 0 0'

    length = 5.0
    n_elems = 10

    names = 'blk'
    widths = '1.0'
    n_part_elems = '2'

    depth = 1.0

    initial_T = ${initial_T}
  []
  [start]
    type = HSBoundarySpecifiedTemperature
    hs = hs
    boundary = hs:start
    T = ${T_left}
  []
  [end]
    type = HSBoundaryAmbientConvection
    hs = hs
    boundary = hs:end
    T_ambient = ${T_right}
    htc_ambient = ${htc_right}
  []
[]

# Currently, there is no way to have a variable of the same name created in THM
# as one in MOOSE, even though they are on different blocks. Thus, we create a
# common variable name here and copy both variables into it for output.
[AuxVariables]
  [T]
  []
[]

[AuxKernels]
  [T_moose_ak]
    type = CopyValueAux
    variable = T
    block = 'hs_moose'
    source = T_moose
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [T_thm_ak]
    type = CopyValueAux
    variable = T
    block = 'hs:blk'
    source = T_solid
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Preconditioning]
  [pc]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  start_time = 0
  dt = 1.0
  num_steps = 5
  abort_on_solve_fail = true

  solve_type = 'NEWTON'
[]

[Outputs]
  [exodus]
    type = Exodus
    file_base = 'mesh_generator_compatibility'
    show = 'T'
  []
[]
