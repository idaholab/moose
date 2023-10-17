# This test solves a 2D steady state heat equation
# The error is found by comparing to the analytical solution

# Note that the thermal conductivity, specific heat, and density in this problem
# Are set to 1, and need to be changed to the constants of the material being
# Analyzed

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 30
  ny = 30
  xmax = 2
  ymax = 2
[]

[Variables]
  [./T]
  [../]
[]

[Kernels]
  [./HeatDiff]
    type = ADHeatConduction
    variable = T
  [../]
[]

[BCs]
  [./zero]
    type = DirichletBC
    variable = T
    boundary = 'right bottom left'
    value = 0
  [../]
  [./top]
    type = ADFunctionDirichletBC
    variable = T
    boundary = top
    function = '10*sin(pi*x*0.5)'
  [../]
[]

[Materials]
  [./properties]
    type = ADGenericConstantMaterial
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '1 1 1'
  [../]
[]

[Postprocessors]
  [./nodal_error]
    type = NodalL2Error
    function = '10/(sinh(pi))*sin(pi*x*0.5)*sinh(pi*y*0.5)'
    variable = T
  [../]
  [./elemental_error]
    type = ElementL2Error
    function = '10/(sinh(pi))*sin(pi*x*0.5)*sinh(pi*y*0.5)'
    variable = T
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
