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

[Functions]
  [./analytical_sol]
    type = ParsedFunction
    value = 10/(sinh(pi))*sin(pi*x*0.5)*sinh(pi*y*0.5)
  [../]
  [./top_bound]
    type = ParsedFunction
    value = 10*sin(pi*x*0.5)
  [../]
[]


[ADKernels]
  [./HeatDiff]
    type = ADHeatConduction
    variable = T
    thermal_conductivity = thermal_conductivity
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = T
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = T
    boundary = right
    value = 0
  [../]
  [./bottom]
    type = DirichletBC
    variable = T
    boundary = bottom
    value = 0
  [../]
  [./top]
    type = FunctionDirichletBC
    variable = T
    boundary = top
    function = top_bound
  [../]
[]

[Materials]
  [./k]
    type = GenericConstantMaterial
    prop_names = thermal_conductivity
    prop_values = 1 # this values is changed based on the material property
    block = 0
  [../]
  [./cp]
    type = GenericConstantMaterial
    prop_names = specific_heat
    prop_values = 1 # this value is changed based on material properties
    block = 0
  [../]
  [./rho]
    type = GenericConstantMaterial
    prop_names = density
    prop_values = 1 # this values is changed based on material properties
    block = 0
  [../]
[]


[Postprocessors]
  [./nodal_error]
    type = NodalL2Error
    function = 'analytical_sol'
    variable = T
  [../]
  [./elemental_error]
    type = ElementL2Error
    function = 'analytical_sol'
    variable = T
  [../]
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-12
  l_tol = 1e-6
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
