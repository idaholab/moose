# Problem III.2
#
# A spherical shell has a thermal conductivity that varies linearly
# with temperature. The inside and outside surfaces of the shell are
# exposed to constant temperatures.
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
    symbol_names = 'ri ro beta ki ko ui uo'
    symbol_values = '0.2 1.0 1e-3 5.3 5 300 0'
    expression = 'uo+(ko/beta)* ( ( 1 + beta*(ki+ko)*(ui-uo)*( (1/x-1/ro) / (1/ri-1/ro) )/(ko^2))^0.5 -1 )'
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = u
  [../]
[]

[BCs]
  [./ui]
    type = DirichletBC
    boundary = left
    variable = u
    value = 300
  [../]
  [./uo]
    type = DirichletBC
    boundary = right
    variable = u
    value = 0
  [../]
[]

[Materials]
  [./property]
    type = GenericConstantMaterial
    prop_names = 'density specific_heat'
    prop_values = '1.0 1.0'
  [../]
  [./thermal_conductivity]
    type = ParsedMaterial
    property_name = 'thermal_conductivity'
    coupled_variables = u
    expression = '5 + 1e-3 * (u-0)'
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
