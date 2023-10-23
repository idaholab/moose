#
# If we assume that epsilon*sigma*(T_inf^4-T_s^4) is approximately equal to
#   epsilon*sigma*4*T_inf^3*(T_inf-T_s), that form is equivalent to
#   h*(T_inf-T_s), the convective flux bc.  So, the radiative and convective
#   flux bcs should give nearly the same answer if the leading terms are equal.
#
[Mesh]
  [top]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    bias_x = 0.8
    ymin = 1.2
    ymax = 2.2
    boundary_name_prefix = top
  []
  [bottom]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 10
    bias_x = 0.8
    boundary_name_prefix = bot
    boundary_id_offset = 6
  []
  [two_blocks]
    type = MeshCollectionGenerator
    inputs = 'top bottom'
  []
[]

[Variables]
  [temp]
    initial_condition = 600.0
  []
[]

[Kernels]
  [heat_dt]
    type = TimeDerivative
    variable = temp
  []
  [heat_conduction]
    type = HeatConduction
    variable = temp
  []
[]

[BCs]
  [./top_right]
    type = ConvectiveHeatFluxBC
    variable = temp
    boundary = top_right
    T_infinity = 300.0
    heat_transfer_coefficient = 3.0
    heat_transfer_coefficient_dT = 0
  [../]
  [./bot_right]
    type = FunctionRadiativeBC
    variable = temp
    boundary = bot_right
    # htc/(stefan-boltzmann*4*T_inf^3)
    emissivity_function = '3/(5.670367e-8*4*300*300*300)'
  [../]
[]

[Materials]
  [./thermal]
    type = GenericConstantMaterial
    prop_names = 'density  thermal_conductivity specific_heat'
    prop_values = '1 10 100'
  [../]
[]

[Postprocessors]
  [./top_left_temp]
    type = SideAverageValue
    variable = temp
    boundary = top_left
    execute_on = 'TIMESTEP_END initial'
  [../]
  [./bot_left_temp]
    type = SideAverageValue
    variable = temp
    boundary = bot_left
    execute_on = 'TIMESTEP_END initial'
  [../]
  [./top_right_temp]
    type = SideAverageValue
    variable = temp
    boundary = top_right
  [../]
  [./bot_right_temp]
    type = SideAverageValue
    variable = temp
    boundary = bot_right
  [../]
[]

[Executioner]
  type = Transient

  num_steps = 10
  dt = 1e1

  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
