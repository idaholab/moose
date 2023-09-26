# This is a simple 1D test of the volumetric heat source with material properties
# of a representative ceramic material.  A bar is uniformly heated, and a temperature
# boundary condition is applied to the left side of the bar.

# Important properties of problem:
# Length: 0.01 m
# Thermal conductivity = 3.0 W/(mK)
# Specific heat = 300.0 J/K
# density = 10431.0 kg/m^3
# Prescribed temperature on left side: 600 K

# When it has reached steady state, the temperature as a function of position is:
#  T = -q/(2*k) (x^2 - 2*x*length) + 600
#  or
#  T = -6.3333e+7 * (x^2 - 0.02*x) + 600
#  on left side: T=600, on right side, T=6933.3

[Mesh]
  type = GeneratedMesh
  dim = 1
  xmax = 0.01
  nx = 20
[]

[Variables]
  [./temp]
    initial_condition = 300.0
  [../]
[]

[Kernels]
  [./heat]
    type = HeatConduction
    variable = temp
  [../]
  [./heatsource]
    type = HeatSource
    function = volumetric_heat
    variable = temp
  [../]
[]

[BCs]
  [./lefttemp]
    type = DirichletBC
    boundary = left
    variable = temp
    value = 600
  [../]
[]

[Materials]
  [./density]
    type = GenericConstantMaterial
    prop_names = 'density  thermal_conductivity'
    prop_values = '10431.0 3.0'
  [../]
[]

[Functions]
  [./volumetric_heat]
     type = ParsedFunction
     expression = 3.8e+8
  [../]
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./right]
    type = SideAverageValue
    variable = temp
    boundary = right
  [../]
  [./error]
    type = NodalL2Error
    function = '-3.8e+8/(2*3) * (x^2 - 2*x*0.01) + 600'
    variable = temp
  [../]
[]

[Outputs]
  execute_on = FINAL
  exodus = true
[]
