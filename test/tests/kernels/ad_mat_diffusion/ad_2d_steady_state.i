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
    type = ADMatDiffusion
    variable = T
    diffusivity = diffusivity
  [../]
[]

[BCs]
  [./zero]
    type = DirichletBC
    variable = T
    boundary = 'left right bottom'
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
  [./k]
    type = ADGenericConstantMaterial
    prop_names = diffusivity
    prop_values = 1
  [../]
[]


[Postprocessors]
  [./nodal_error]
    type = NodalL2Error
    function = '10/(sinh(pi))*sin(pi*x*0.5)*sinh(pi*y*0.5)'
    variable = T
    outputs = console
  [../]
  [./elemental_error]
    type = ElementL2Error
    function = '10/(sinh(pi))*sin(pi*x*0.5)*sinh(pi*y*0.5)'
    variable = T
    outputs = console
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
