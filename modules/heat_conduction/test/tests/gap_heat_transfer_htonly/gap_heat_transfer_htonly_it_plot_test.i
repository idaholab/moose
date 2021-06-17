#
# 1-D Gap Heat Transfer Test without mechanics
#
# This test exercises 1-D gap heat transfer for a constant conductivity gap.
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
#  Flux = (T_left - T_right) * (gapK/gap_width)
#
# The gap conductivity is specified as 1, thus
#
#  gapK(Tavg) = 1.0*Tavg
#
#
# The heat flux across the gap at time = 2 is then:
#
#  Flux(2) = 100 * (1.0/1.0) = 100
#
# For comparison, see results from the flux post processors
#


[Mesh]
  file = gap_heat_transfer_htonly_test.e
[]

[Functions]

  [./temp]
    type = PiecewiseLinear
    x = '0   1   2'
    y = '100 200 200'
  [../]
[]

[ThermalContact]
  [./thermal_contact]
    type = GapHeatTransfer
    variable = temp
    primary = 3
    secondary = 2
    emissivity_primary = 0
    emissivity_secondary = 0
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

[Materials]

  [./heat1]
    type = HeatConductionMaterial
    block = '1 2'
    specific_heat = 1.0
    thermal_conductivity = 100000000.0
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'




  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'


  line_search = 'none'


  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-12

  l_tol = 1e-10
  l_max_its = 100

  start_time = 0.0
  dt = 1e-1
  end_time = 2.0
[]

[Postprocessors]

  [./temp_left]
    type = SideAverageValue
    boundary = 2
    variable = temp
  [../]

  [./temp_right]
    type = SideAverageValue
    boundary = 3
    variable = temp
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
  file_base = out_it_plot
  [./exodus]
    type = Exodus
    execute_on = 'initial timestep_end nonlinear'
    nonlinear_residual_dt_divisor = 100
  [../]
[]
