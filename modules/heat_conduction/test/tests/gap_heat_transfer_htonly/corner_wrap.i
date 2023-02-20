[Mesh]
  file = corner_wrap.e
[]

[ThermalContact]
  [thermal_contact]
    type = GapHeatTransfer
    variable = temp
    primary = 3
    secondary = 2
    emissivity_primary = 0
    emissivity_secondary = 0
    check_boundary_restricted = false
    quadrature = true
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
  [temp_bot_right]
    type = DirichletBC
    boundary = 1
    variable = temp
    value = 50
  []
  [temp_top_left]
    type = DirichletBC
    boundary = 4
    variable = temp
    value = 100
  []
[]

[Materials]
  [heat1]
    type = HeatConductionMaterial
    block = '1 2'
    specific_heat = 1.0
    thermal_conductivity = 1.0
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  line_search = 'none'
  nl_rel_tol = 1e-14
  l_tol = 1e-3
  l_max_its = 100
#  dt = 1e-1
#  end_time = 1.0
[]

[Outputs]
  exodus = true
[]
