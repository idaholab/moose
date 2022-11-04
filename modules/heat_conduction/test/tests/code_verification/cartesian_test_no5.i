# Problem I.5
#
# The volumetric heat generation in an infinite plate varies linearly
# with spatial location. It has constant thermal conductivity.
# It is insulated on the left boundary and exposed to a
# constant temperature on the right.
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
  [./volumetric_heat]
    type = ParsedFunction
    symbol_names = 'q L beta'
    symbol_values = '1200 1 0.1'
    expression = 'q * (1-beta*x/L)'
  [../]
  [./exact]
    type = ParsedFunction
    symbol_names = 'uo q k L beta'
    symbol_values = '300 1200 1 1 0.1'
    expression = 'uo + (0.5*q*L^2/k) * ( (1-(x/L)^2) - (1-(x/L)^3) * beta/3 )'
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
