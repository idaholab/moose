
[GlobalParams]
  order = SECOND
  family = LAGRANGE
[]

[Mesh]
  coord_type = RZ
  [fred]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 25
    xmin = 0
    xmax = 1
    boundary_name_prefix = left
    elem_type = edge3
  []
  [wilma]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 25
    xmin = 2
    xmax = 3
    boundary_id_offset = 10
    boundary_name_prefix = right
    elem_type = edge3
  []
  [combine]
    type = CombinerGenerator
    inputs = 'fred wilma'
  []
[]

[Functions]
  [temp]
    type = PiecewiseLinear
    x = '0   1   2'
    y = '100 200 200'
  []
[]

[Variables]
  [temp]
    initial_condition = 100
  []
[]

[Kernels]
  [heat]
    type = HeatConduction
    variable = temp
  []
[]

[BCs]
  [temp_far_left]
    type = FunctionDirichletBC
    boundary = left_left
    variable = temp
    function = temp
  []

  [temp_far_right]
    type = DirichletBC
    boundary = right_right
    variable = temp
    value = 100
  []
[]

[ThermalContact]
  [thermal_contact]
    type = GapHeatTransfer
    variable = temp
    primary = right_left
    secondary = left_right
    emissivity_primary = 0
    emissivity_secondary = 0
  []
[]

[Materials]
  [heat]
    type = HeatConductionMaterial
    specific_heat = 1.0
    thermal_conductivity = 1.0e6
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'

  nl_abs_tol = 1e-3
  nl_rel_tol = 1e-12

  l_tol = 1e-8
  l_max_its = 100

  start_time = 0.0
  dt = 2e-1
  end_time = 2.0
[]

[Postprocessors]
  [temp_left]
    type = SideAverageValue
    boundary = left_right
    variable = temp
    execute_on = 'initial timestep_end'
  []

  [temp_right]
    type = SideAverageValue
    boundary = right_left
    variable = temp
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  exodus = true
[]
