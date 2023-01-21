[Debug]
  show_var_residual = 'T'
  show_var_residual_norms = true
[]

[GlobalParams]
  order = SECOND
  family = LAGRANGE
[]

[Mesh]
  [left] # block = 0; In 2D, bottom = 0, right = 1, top = 2, left = 3
    type = GeneratedMeshGenerator
    dim = 2
    elem_type = QUAD8
    xmin = -0.0105
    xmax = -0.0005
    nx = 5
    ymin = -0.005
    ymax = 0.005
    ny = 5
  []

  [right_form]
    type = GeneratedMeshGenerator
    dim = 2
    elem_type = QUAD8
    xmin = 0.0005
    xmax = 0.0105
    nx = 5
    ymin = -0.015
    ymax = 0.015
    ny = 10
  []
  [rename_right]
    type = RenameBlockGenerator
    input = right_form
    old_block_id = 0
    new_block_id = 1
  []
  [collect_mesh]
    type = MeshCollectionGenerator
    inputs = 'left rename_right'
  []
  [side_bot2]
    type = SideSetsAroundSubdomainGenerator
    input = collect_mesh
    block = 1
    new_boundary = '4'
    normal = '0 -1 0'
    replace = true
  []
  [side_right2]
    type = SideSetsAroundSubdomainGenerator
    input = side_bot2
    block = 1
    new_boundary = '5'
    normal = '1 0 0'
    replace = true
  []
  [side_top2]
    type = SideSetsAroundSubdomainGenerator
    input = side_right2
    block = 1
    new_boundary = '6'
    normal = '0 1 0'
    replace = true
  []
  [side_left2]
    type = SideSetsAroundSubdomainGenerator
    input = side_top2
    block = 1
    new_boundary = '7'
    normal = '-1 0 0'
    replace = true
  []
[]

[Functions]
  [left_heat]
    type = PiecewiseLinear
    x = '0 0.1 1'
    y = '1 1e12 1e6'
  []
[]

[Variables]
  [T]
    initial_condition = 273.0
  []
[]

[Kernels]
  [heat]
    type = HeatConduction
    variable = T
  []
  [heat_ie]
    type = HeatConductionTimeDerivative
    variable = T
  []
  [heat_source_right]
    type = HeatSource
    block = 0 # left
    variable = T
    function = left_heat
  []
[]

[BCs]
  [right_right]
    type = DirichletBC
    boundary = 5
    variable = T
    value = 293
  []
[]

[ThermalContact]
  [connect_T]
    type = GapHeatTransfer
    variable = T
    primary = 7
    secondary = right
    quadrature = true
    gap_conductivity = 74.0
    min_gap = 1e-3
  []
[]

[Postprocessors]
  [_dt]
    type = TimestepSize
    execute_on = 'initial timestep_end'
  []
  [avg_left]
    type = ElementAverageValue
    block = 0
    variable = T
    execute_on = 'initial timestep_end'
  []
  [avg_right]
    type = ElementAverageValue
    block = 1
    variable = T
    execute_on = 'initial timestep_end'
  []
  [max_left]
    type = ElementExtremeValue
    block = 0
    value_type = max
    variable = T
    execute_on = 'initial timestep_end'
  []
  [min_right]
    type = ElementExtremeValue
    block = 1
    value_type = min
    variable = T
    execute_on = 'initial timestep_end'
  []
  [avg_side_near_right]
    type = SideAverageValue
    boundary = right
    variable = T
    execute_on = 'initial timestep_end'
  []
  [avg_side_near_left]
    type = SideAverageValue
    boundary = 7
    variable = T
    execute_on = 'initial timestep_end'
  []
  [max_side_near_right]
    type = NodalExtremeValue
    boundary = right
    value_type = max
    variable = T
    execute_on = 'initial timestep_end'
  []
  [min_side_near_right]
    type = NodalExtremeValue
    boundary = 7
    value_type = min
    variable = T
    execute_on = 'initial timestep_end'
  []
  [peak_left]
    type = TimeExtremeValue
    postprocessor = max_left
    value_type = max
    execute_on = 'initial timestep_end'
  []
  [valley_right]
    type = TimeExtremeValue
    postprocessor = min_right
    value_type = min
    execute_on = 'initial timestep_end'
  []
[]

[Materials]
  [left_props]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'density thermal_conductivity specific_heat'
    prop_values = '7850 44.5 475'
  []
  [right_props]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'density thermal_conductivity specific_heat'
    prop_values = '7850 44.5 475'
  []
[]

[Preconditioning]
  [SMP]
    type = SMP
    full = true
    solve_type = 'PJFNK'
    petsc_options = '-snes_ksp_ew -snes_converged_reason'
    petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart -pc_hypre_boomeramg_max_iter -pc_hypre_boomeramg_tol'
    petsc_options_value = 'hypre boomeramg 101 20 1.0e-6'
  []
[]

[Executioner]
  type = Transient
  nl_rel_tol = 1e-6
  nl_abs_tol = 1e-9
  start_time = 0.0
  end_time = 1
  dtmin = 1e-6
  dtmax = 0.1

  [TimeStepper]
    type = ConstantDT
    dt = 0.01
  []
[]

[Outputs]
  perf_graph = true
  interval = 1
  exodus = true
  csv = true
  [console]
    type = Console
    output_linear = true
    output_nonlinear = true
  []
[]
# vi:filetype=moose_fw
