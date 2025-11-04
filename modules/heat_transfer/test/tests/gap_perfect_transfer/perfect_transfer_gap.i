#
# 1-D Gap Perfect Heat Transfer
#
# The mesh consists of two element blocks containing one element each.  Each
#   element is a unit line.  They sit next to one another with a unit between
#   them.
#
# The temperature of the far left boundary is ramped from 100 to 200 over one
#   second and then held fixed.  The temperature of the far right boundary
#   follows due to the perfect heat transfer.
#

[Mesh]
  [left]
    type = GeneratedMeshGenerator
    dim = 1
    boundary_name_prefix = left
  []
  [right]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 2
    xmax = 3
    boundary_name_prefix = right
    boundary_id_offset = 2
  []
  [right_block]
    type = SubdomainIDGenerator
    input = right
    subdomain_id = 1
  []
  [collect]
    type = CombinerGenerator
    inputs = 'left right_block'
  []
[]

[Functions]
  [temperature]
    type = PiecewiseLinear
    x = '0   1   2'
    y = '100 200 200'
  []
[]

[Variables]
  [temperature]
    order = FIRST
    family = LAGRANGE
    initial_condition = 100
  []
[]

[Kernels]
  [heat]
    type = HeatConduction
    variable = temperature
  []
[]

[BCs]
  [temp_far_left]
    type = FunctionDirichletBC
    boundary = 0
    variable = temperature
    function = temperature
  []
[]

[ThermalContact]
  [thermal_contact_1]
    type = GapPerfectConductance
    penalty = 1e3
    variable = temperature
    primary = 1
    secondary = 2
  []
[]

[Materials]
  [heat1]
    type = HeatConductionMaterial
    block = 0
    specific_heat = 1.0
    thermal_conductivity = 1.0
  []
  [heat2]
    type = HeatConductionMaterial
    block = 1
    specific_heat = 1.0
    thermal_conductivity = 10.0
  []
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu superlu_dist'

  line_search = 'none'

  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-14

  l_tol = 1e-3
  l_max_its = 100

  start_time = 0.0
  dt = 1e-1
  end_time = 2.0
  num_steps = 50
[]

[Postprocessors]
  [aveTempLeft]
    type = SideAverageValue
    boundary = 0
    variable = temperature
    execute_on = 'initial timestep_end'
  []
  [aveTempRight]
    type = SideAverageValue
    boundary = 3
    variable = temperature
    execute_on = 'initial timestep_end'
  []
[]

[Outputs]
  csv = true
[]
