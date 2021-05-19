#
# 1-D Gap Heat Transfer Test without mechanics
#
# This test exercises 1-D gap heat transfer for a helium-filled gap including radiation.
#
# The mesh consists of two element blocks containing one element each.  Each
#   element is a unit cube.  They sit next to one another with a unit between them.
#
# The conductivity of both blocks is set very large to achieve a uniform temperature
#  across each block. The temperature of the far left boundary
#  is ramped from 100 to 200 over one time unit, and then held fixed for an additional
#  time unit.  The temperature of the far right boundary is held fixed at 100.
#
# A simple analytical solution is possible for the heat flux between the blocks:
#
#  Flux = (T_left - T_right) * h_gap
#
#    where  h_gap = h_gas + h_cont + h_rad
#
# By setting the contact pressure, roughnesses, and jump distances to zero, the gap
#   conductance simplifies to:
#
#    h_gap = gapK/d_gap + sigma*Fe*(T_left^2 + T_right^2)*(T_left + T_right)
#
#      where Fe = 1/(1/eps_left + 1/eps_right - 1)
#            eps = emissivity
#
# For pure helium, BISON computes the gas conductivity as:
#
#  gapK(Tavg) = 2.639e-3*Tavg^0.7085
#
# For the test, the final (t=2) average gas temperature is (200 +100)/2 = 150,
#  giving gapK(150) = 0.09187557
#
# Assuming ems_left = ems_right = 0.5, Fe = 1/3
#
# The heat flux across the gap at that time is then:
#
#  Flux(2) = 100 * ((0.09187557/1.0) + (5.669e-8/3)*(200^2 + 100^2)*(200 + 100))
#          = 37.532557
#
# The flux post processors give 37.53255
#

[Mesh]
  file = gap_heat_transfer_radiation_test.e
[]

[Functions]
  [./temp]
    type = PiecewiseLinear
    x = '0   1'
    y = '200 200'
  [../]
[]

[Variables]
  [./temp]
    order = FIRST
    family = LAGRANGE
    initial_condition = 100
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]


[BCs]
  [./temp_far_left]
    type = FunctionDirichletBC
    boundary = 1
    variable = temp
    function = temp
  [../]

  [./temp_far_right]
    type = DirichletBC
    boundary = 4
    variable = temp
    value = 100
  [../]
[]

[ThermalContact]
  [./gap]
    type = GapHeatTransfer
    variable = temp
    primary = 3
    secondary = 2
    gap_conductivity = 0.09187557
    emissivity_primary = 0.5
    emissivity_secondary = 0.5
  [../]
[]

[Materials]
  [./heat1]
    type = HeatConductionMaterial
    block = '1 2'
    specific_heat = 1.0
    thermal_conductivity = 10000000.0
  [../]
  [./density]
    type = GenericConstantMaterial
    block = '1 2'
    prop_names = 'density'
    prop_values = '1.0'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'

  nl_abs_tol = 1e-6
  nl_rel_tol = 1e-10

  l_tol = 1e-3
  l_max_its = 100

  start_time = 0.0
  dt = 1
  end_time = 1.0
[]

[Postprocessors]
  [./temp_left]
    type = SideAverageValue
    boundary = 2
    variable = temp
    execute_on = 'initial timestep_end'
  [../]

  [./temp_right]
    type = SideAverageValue
    boundary = 3
    variable = temp
    execute_on = 'initial timestep_end'
  [../]

  [./flux_left]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 2
    diffusivity = thermal_conductivity
  [../]

  [./flux_right]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 3
    diffusivity = thermal_conductivity
  [../]
[]

[Outputs]
  exodus = true
[]
