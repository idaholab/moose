# Problem II.3
#
# The thermal conductivity of an infinitely long hollow cylinder varies
# linearly with temperature: k = k0(1+beta*u). The tube inside radius is ri and
# outside radius is ro. It has a constant internal heat generation q and
# is exposed to the same constant temperature on both surfaces: u(ri) = u(ro) = uo.
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
    symbol_names = 'q k0 ri ro beta u0'
    symbol_values = '1200 1 0.2 1.0 1e-3 0'
    expression = 'u0+(1/beta)*( ( 1 + 0.5*beta*((ro^2-x^2)-(ro^2-ri^2) * log(ro/x)/log(ro/ri))*q/k0 )^0.5  - 1)'
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
  [./uo]
    type = DirichletBC
    boundary = 'left right'
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
    expression = '1 * (1 + 1e-3*u)'
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
