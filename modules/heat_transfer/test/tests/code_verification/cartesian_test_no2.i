# Problem I.2
#
# An infinite plate with a thermal conductivity that varies linearly with
# temperature. Each boundary is exposed to a constant temperature.
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
    symbol_names = 'L beta ki ko ui uo'
    symbol_values = '1 1e-3 5.3 5 300 0'
    expression = 'uo+(ko/beta)* ( (1 + L*beta*(ki+ko)*(ui-uo)*((L-x)/(ko*L)^2) )^0.5  - 1)'
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
