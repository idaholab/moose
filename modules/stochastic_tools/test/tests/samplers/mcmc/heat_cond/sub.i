# Problem I.1
#
# An infinite plate with constant thermal conductivity k and
# internal heat generation q. It is exposed on each boundary
# to a constant temperature: u(0) = ui and u(L) = uo.
#
# REFERENCE:
# A. Toptan, et al. (Mar.2020). Tech. rep. CASL-U-2020-1939-000, SAND2020-3887 R. DOI:10.2172/1614683.

k1 = 12.0
q1 = 1200
bc1 = 100

param1 = '${fparse k1}'
param2 = '${fparse q1}'
param3 = '${fparse bc1}'

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
    vars = 'q L k ui uo'
    vals = '1200 1 12 100 0'
    value = 'ui + (uo-ui)*x/L + (q/k) * x * (L-x) / 2'
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = u
  [../]
  [./heatsource]
    type = HeatSource
    function = ${param2}
    variable = u
  [../]
[]

[BCs]
  [./ui]
    type = DirichletBC
    boundary = left
    variable = u
    value = ${param3}
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
    prop_values = '1.0 1.0 ${param1}'
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
  console = false
[]
