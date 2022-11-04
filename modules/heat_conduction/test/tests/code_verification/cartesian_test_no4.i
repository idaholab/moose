# Problem I.4
#
# An infinite plate with constant thermal conductivity k and internal
# heat generation q. The left boundary is exposed to a constant heat flux q0.
# The right boundary is exposed to a fluid with constant temperature uf and
# heat transfer coefficient h, which results in the convective boundary condition.
#
# REFERENCE:
# A. Toptan, et al. (Mar.2020). Tech. rep. CASL-U-2020-1939-000, SAND2020-3887 R. DOI:10.2172/1614683.

[Mesh]
  [./geom]
    type = GeneratedMeshGenerator
    dim = 1
    elem_type = EDGE2
    nx = 1
  [../]
[]

[Variables]
  [./u]
    order = FIRST
  [../]
[]

[Functions]
  [./exact]
    type = ParsedFunction
    symbol_names = 'q q0 k L uf h'
    symbol_values = '1200 200 1 1 100 10.0'
    expression = 'uf + (q0 + L * q)/h + 0.5 * ( 2 * q0 + q * (L + x)) * (L-x) / k'
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = u
  [../]
  [./heatsource]
    type = HeatSource
    function = 1200
    variable = u
  [../]
[]

[BCs]
  [./ui]
    type = NeumannBC
    boundary = left
    variable = u
    value = 200
  [../]
  [./uo]
    type = CoupledConvectiveHeatFluxBC
    boundary = right
    variable = u
    htc = 10.0
    T_infinity = 100
  [../]
[]

[Materials]
  [./property]
    type = GenericConstantMaterial
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '1.0 1.0 1.0'
  [../]
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./error]
    type = ElementL2Error
    function = exact
    variable = u
  [../]
  [./h]
    type = AverageElementSize
  []
[]

[Outputs]
  csv = true
[]
