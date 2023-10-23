#
# 1-D spherical Gap Heat Transfer Test
#
# This test exercises 1-D gap heat transfer for a constant conductivity gap.
#
# The mesh consists of two "blocks" with a mesh biased toward the gap
#   between them.  Each block is unit length.  The gap between them is one
#   unit in length.
#
# The conductivity of both blocks is set very large to achieve a uniform temperature
#  across each block. The temperature of the far left boundary
#  is ramped from 100 to 200 over one time unit, and then held fixed for an additional
#  time unit.  The temperature of the far right boundary is held fixed at 100.
#
# A simple analytical solution is possible for the heat flux between the blocks, or spheres in the case of RSPHERICAL.:
#
#  Flux = (T_left - T_right) * (gapK/(r^2*((1/r1)-(1/r2))))
#
# For gapK = 1 (default value)
#
# The area is taken as the area of the secondary (inner) surface:
#
# Area = 4 * pi * 1 * 1
#
# The integrated heat flux across the gap at time 2 is then:
#
# 4*pi*k*delta_T/((1/r1)-(1/r2))
# 4*pi*1*100/((1/1) - (1/2)) =  2513.3 watts
#
# For comparison, see results from the flux post processors.
#
#

[Problem]
  coord_type = RSPHERICAL
[]

[Mesh]
  file = gap_heat_transfer_htonly_rspherical.e
  construct_side_list_from_node_list = true
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
    gap_geometry_type = sphere
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
    thermal_conductivity = 1e6
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

  # I don't know enough about this test to say why it needs such a
  # loose nl_abs_tol... after timestep 10 the residual basically can't
  # be reduced much beyond the initial residual.  The test probably
  # needs to be revisited to determine why.
  nl_abs_tol = 1e-3
  nl_rel_tol = 1e-10
  l_tol = 1e-6
  l_max_its = 100
  line_search = 'none'
  nl_max_its = 10

  dt = 1e-1
  dtmin = 1e-1
  end_time = 2.0
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
