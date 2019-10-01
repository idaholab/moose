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
  xmax = 0.5
  ymax = 0.25
[]

[Variables]
  [./T]
  [../]
[]

[Kernels]
  [./HeatDiff]
    type = ADHeatConduction
    variable = T
    axis_scaling_vector = '0.25 0.125 0'
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
    function = '10*sin(pi*x*2)'
  [../]
[]

[Materials]
  [./properties]
    type = GenericConstantMaterial
    prop_names = 'thermal_conductivity specific_heat density'
    prop_values = '1 1 1'
  [../]
[]

[Postprocessors]
  [./nodal_error]
    type = NodalL2Error
    function = '10/(sinh(pi))*sin(pi*x*2)*sinh(pi*y*4)'
    variable = T
  [../]
  [./elemental_error]
    type = ElementL2Error
    function = '10/(sinh(pi))*sin(pi*x*2)*sinh(pi*y*4)'
    variable = T
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
