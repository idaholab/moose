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
    x = '0 39600 86400'
    y = '25 100 0'
  []
  [left_left_flux]
    type = PiecewiseLinear
    x = '0 82800 86400'
    y = '-50 -50 0'
  []
  [right_cool]
    type = PiecewiseLinear
    x = '0 46800 86400'
    y = '25 0 -20000'
  []
  [right_left_T]
    type = PiecewiseLinear
    x = '0 86400'
    y = '200 1000'
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
  [heat_source_left]
    type = HeatSource
    block = 0 # left
    variable = T
    function = left_heat
  []
  [heat_sink_right]
    type = HeatSource
    block = 1 # right
    variable = T
    function = right_cool
  []
[]

[BCs] # If not specified, defaults to Neumann = 0
  [left_top]
    type = DirichletBC
    boundary = top
    variable = T
    value = 293.0
  []
  [left_left]
    type = FunctionNeumannBC
    boundary = bottom
    variable = T
    function = left_left_flux
  []
  [right_left]
    type = FunctionDirichletBC
    boundary = 7
    variable = T
    function = right_left_T
  []
[]

[CoolantChannel]
  [convective_right_right]
    boundary = 5
    variable = T
    inlet_temperature = 600.0
    inlet_pressure = 455054.0
    inlet_massflux = 4520.72
    coolant_material = sodium
    flow_area = 2.22e-5
    hydraulic_diameter = 2.057e-3
    heated_perimeter = 1.835e-2
    heated_diameter = 4.84e-3
    number_axial_zone = 6
    htc_correlation_type = 3
    compute_enthalpy = true
  []
[]

[AuxVariables]
  [coolant_T]
    order = CONSTANT
    family = MONOMIAL
    block = 1
    initial_condition = 600.0
  []
[]

[AuxKernels]
  [coolant_temperature]
    type = MaterialRealAux
    property = coolant_temperature
    variable = coolant_T
    boundary = 5
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
  [min_coolant]
    #type = NodalExtremeValue
    #boundary = 5
    type = ElementExtremeValue
    block = 1
    value_type = min
    variable = coolant_T
    execute_on = 'initial timestep_end'
  []
  [avg_coolant]
    type = SideAverageValue
    boundary = 5
    variable = coolant_T
    execute_on = 'initial timestep_end'
  []
[]

[Materials]
  [left_props]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'density thermal_conductivity specific_heat'
    prop_values = '7850 4.45 4750'
  []
  [right_props]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'density thermal_conductivity specific_heat'
    prop_values = '7850 10.5 4750'
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
  end_time = 86400 # 24 hours
  dtmin = 30.0
  dtmax = 3600.0

  [TimeStepper]
    type = ConstantDT
    dt = 300
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
