# Problem II.5
#
# The volumetric heat generation in an infinitely long solid cylinder
# varies with spatial location. It has a constant thermal conductivity.
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

[Problem]
  coord_type = RZ
[]

[Functions]
  [./volumetric_heat]
    type = ParsedFunction
    symbol_names = 'q ro beta'
    symbol_values = '1200 1 0.1'
    expression = 'q * (1-beta*x/ro)'
  [../]
  [./exact]
    type = ParsedFunction
    symbol_names = 'uo q k ro beta'
    symbol_values = '300 1200 1 1 0.1'
    expression = 'uo + (0.25*q*ro^2/k) * ( (1-(x/ro)^2) - (1-(x/ro)^3) * beta * 4/9 )'
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = u
  [../]
  [./heatsource]
    type = HeatSource
    function = volumetric_heat
    variable = u
  [../]
[]

[BCs]
  [./uo]
    type = DirichletBC
    boundary = right
    variable = u
    value = 300
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
