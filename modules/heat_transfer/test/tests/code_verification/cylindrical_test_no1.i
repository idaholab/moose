# Problem II.1
#
# An infinitely long hollow cylinder has an inner radius ri and
# outer radius ro. It has a constant thermal conductivity k and
# internal heat generation q. It is allowed to reach thermal
# equilibrium while being exposed to constant temperatures on its
# inside and outside boundaries: u(ri) = ui and u(ro) = uo.
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
    symbol_names = 'ri ro ui uo'
    symbol_values = '0.2 1.0 300 0'
    expression = '( uo * log(ri) - ui * log(ro) + (ui-uo) * log(x) ) / log(ri/ro)'
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
    prop_names = 'density specific_heat thermal_conductivity'
    prop_values = '1.0 1.0 5.0'
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
