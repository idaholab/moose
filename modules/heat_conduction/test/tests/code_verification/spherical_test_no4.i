# Problem III.4
#
# A spherical shell has thermal conductivity k and heat generation q.
# It has an inner radius ri and outer radius ro. A constant heat flux is
# applied to the inside surface qin and the outside surface is exposed
# to a fluid temperature uf and heat transfer coefficient h.
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
  coord_type = RSPHERICAL
[]

[Functions]
  [./exact]
    type = ParsedFunction
    symbol_names = 'qin q k ri ro uf h'
    symbol_values = '100 1200 1.0 0.2 1 100 10'
    expression = 'uf+ (q/(6*k)) * ( ro^2-x^2 + 2*k*(ro^3-ri^3)/(h*ro^2) + 2 * ri^3 * (1/ro-1/x) ) + (1/x-1/ro+k/(h*ro^2)) * qin * ri^2 / k'
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
