# 1-D Gap Heat Transfer Test without mechanics
#
# This test exercises 1-D gap heat transfer for a constant conductivity gap.
#
# The mesh consists of two element blocks in the x-y plane.  Each element block
# is a square. They sit next to one another with a unit between them.
#
# The conductivity of both blocks is set very large to achieve a uniform temperature
# across each block. The temperature of the far bottom boundary
# is ramped from 100 to 200 over one time unit.  The temperature of the far top
# boundary is held fixed at 100.
#
# A simple analytical solution is possible for the heat flux between the blocks:
#
# Flux = (T_left - T_right) * (gapK/gap_width)
#
# The gap conductivity is specified as 1, thus
#
# gapK(Tavg) = 1.0*Tavg
#
# The heat flux across the gap at time = 1 is then:
#
# Flux = 100 * (1.0/1.0) = 100
#
# For comparison, see results from the flux post processors.  These results
# are the same as for the unit 1-D gap heat transfer between two unit cubes.



[Mesh]
  file = simple_2D.e
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

[AuxVariables]
  [./gap_cond]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
[]


[BCs]
  [./temp_far_bottom]
    type = FunctionDirichletBC
    boundary = 1
    variable = temp
    function = temp
  [../]
  [./temp_far_top]
    type = DirichletBC
    boundary = 4
    variable = temp
    value = 100
  [../]
[]

[AuxKernels]
  [./conductance]
    type = MaterialRealAux
    property = gap_conductance
    variable = gap_cond
    boundary = 2
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

  solve_type = 'PJFNK'
  petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_hypre_type -pc_hypre_boomeramg_max_iter'
  petsc_options_value = '201                hypre    boomeramg      4'

  line_search = 'none'
  nl_rel_tol = 1e-14
  l_tol = 1e-3
  l_max_its = 100

  dt = 1e-1
  end_time = 1.0
[]

[Postprocessors]
  [./temp_bottom]
    type = SideAverageValue
    boundary = 2
    variable = temp
    execute_on = 'initial timestep_end'
  [../]

  [./temp_top]
    type = SideAverageValue
    boundary = 3
    variable = temp
    execute_on = 'initial timestep_end'
  [../]

  [./flux_bottom]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 2
    diffusivity = thermal_conductivity
    execute_on = 'initial timestep_end'
  [../]

  [./flux_top]
    type = SideDiffusiveFluxIntegral
    variable = temp
    boundary = 3
    diffusivity = thermal_conductivity
    execute_on = 'initial timestep_end'
  [../]
[]

[Outputs]
  exodus = true
[]
