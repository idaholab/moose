#
# RZ Gap Heat Transfer Test without mechanics
#
#
# The mesh consists of two element blocks containing ten elements each.  Each
#   element is a unit cube.  They sit next to one another with a unit between them.
#
# The conductivity of both blocks is large to achieve a uniform temperature
#  across each block. The temperature of the far left boundary
#  is ramped from 100 to 200 over one time unit, and then held fixed for an additional
#  time unit.  The temperature of the far right boundary is held fixed at 100.
#
# A simple analytical solution is possible for the heat flux between the blocks, or cylinders in the case of RZ.:
#
#  Flux = (T_left - T_right) * (gapK/(r*ln(r2/r1)))
#
# For gapK = 1 (default value)
#
# The integrated heat flux across the gap at time 2 is then:
#
# 2*pi*h*k*delta_T/(ln(r2/r1))
# 2*pi*1*1*100/(ln(2/1)) = 906.5 watts
#
# For comparison, see results from the flux post processors
#
[GlobalParams]
  # Set initial fuel density, other global parameters
  order = SECOND
  family = LAGRANGE
[]


[Mesh]
  file = mesh_quad8.e
[]

[Problem]
  coord_type = RZ
[]

[Functions]

  [./temp]
    type = PiecewiseLinear
    x = '0   1   2'
    y = '100 200 200'
  [../]
[]

[Variables]
  [./temp]
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
    type = FunctionPresetBC
    boundary = 1
    variable = temp
    function = temp
  [../]

  [./temp_far_right]
    type = PresetBC
    boundary = 4
    variable = temp
    value = 100
  [../]
[]

[ThermalContact]
  [./thermal_contact]
    type = GapHeatTransfer
    variable = temp
    master = 3
    slave = 2

    quadrature = true
    cylindrical_gap = true
  [../]
[]

[Materials]

  [./heat1]
    type = HeatConductionMaterial
    block = '1 2'
    specific_heat = 1.0
    thermal_conductivity = 100000000.0
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'




  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'


  line_search = 'none'


  nl_abs_tol = 9e-2
  nl_rel_tol = 1e-12

  l_tol = 1e-8
  l_max_its = 100

  start_time = 0.0
  dt = 2e-1
  end_time = 2.0

  [./Quadrature]
    order = THIRD
  [../]

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
    type = SideFluxIntegral
    variable = temp
    boundary = 2
    diffusivity = thermal_conductivity
  [../]

  [./flux_right]
    type = SideFluxIntegral
    variable = temp
    boundary = 3
    diffusivity = thermal_conductivity
  [../]


[Outputs]
  file_base = cylindrical_out
  output_initial = true
  exodus = true
  [./console]
    type = Console
    perf_log = true
    linear_residuals = true
  [../]
[]
