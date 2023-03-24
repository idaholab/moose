# Problem I.3
#
# The thermal conductivity of an infinite plate varies linearly with
# temperature: k = ko(1+beta*u). It has a constant internal heat generation q,
# and has the boundary conditions du/dx = 0 at x= L and u(L) = uo.
#
# REFERENCE:
# A. Toptan, et al. (Mar.2020). Tech. rep. CASL-U-2020-1939-000, SAND2020-3887 R. DOI:10.2172/1614683.

[Mesh]
  [./geom]
    type = GeneratedMeshGenerator
    dim = 1
    elem_type = EDGE2
    nx = 4
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
    symbol_names = 'q L beta uo ko'
    symbol_values = '1200 1 1e-3 0 1'
    expression = 'uo+(1/beta)*( ( 1 + (1-(x/L)^2) * (beta*q*L^2) / ko )^0.5  - 1)'
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
    value = 0
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
