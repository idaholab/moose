# Problem II.4
#
# An infinitely long hollow cylinder has thermal conductivity k and internal
# heat generation q. Its inner radius is ri and outer radius is ro.
# A constant heat flux is applied to the inside surface qin and
# the outside surface is exposed to a fluid temperature T and heat transfer
# coefficient h, which results in the convective boundary condition.
#
# REFERENCE:
# A. Toptan, et al. (Mar.2020). Tech. rep. CASL-U-2020-1939-000, SAND2020-3887 R. DOI:10.2172/1614683.

[Mesh]
  [./geom]
    type = GeneratedMeshGenerator
    dim = 1
    elem_type = EDGE2
    xmin = 0.2
    nx = 4
  [../]
[]

[Variables]
  [./u]
    order = FIRST
  [../]
[]

[Problem]
  coord_type = RZ
[]

[Functions]
  [./exact]
    type = ParsedFunction
    symbol_names = 'qin q k ri ro uf h'
    symbol_values = '100 1200 1.0 0.2 1 100 10'
    expression = 'uf+ (0.25*q/k) * ( 2*k*(ro^2-ri^2)/(h*ro) + ro^2-x^2 + 2*ri^2*log(x/ro)) + (k/(h*ro) - log(x/ro)) * qin * ri / k'
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
    value = 100
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
