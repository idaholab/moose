# Test of HenryGasConstantScalarAux for different temperatures
# This test is for FLiBe and two different noble gases: helium and argon
# Gold values were digitized from:
#   K. Lee, et al., "Semi-empirical model for Henry's law constant of noble gases in molten salt",
#   Scientific Reports (2024) 14:12847, https://doi.org/10.1038/s41598-024-60006-9.
#
# Ensure that all "check_" values evaluate to 1

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmin = 0
  xmax = 1
[]

[AuxVariables]
  [helium_henry_750]
    family = SCALAR
  []
  [helium_henry_900]
    family = SCALAR
  []
  [helium_henry_1100]
    family = SCALAR
  []
  [argon_henry_750]
    family = SCALAR
  []
  [argon_henry_900]
    family = SCALAR
  []
  [argon_henry_1100]
    family = SCALAR
  []
[]

[AuxScalarKernels]
  [helium_henry_aux_750]
    type = HenryGasConstantScalarAux
    T = 750 # K
    radius = 1.40e-10 # m helium
    salt = flibe
    variable = helium_henry_750
  []
  [helium_henry_aux_900]
    type = HenryGasConstantScalarAux
    T = 900 # K
    radius = 1.40e-10 # m helium
    salt = flibe
    variable = helium_henry_900
  []
  [helium_henry_aux_1100]
    type = HenryGasConstantScalarAux
    T = 1100 # K
    radius = 1.40e-10 # m helium
    salt = flibe
    variable = helium_henry_1100
  []
  [argon_henry_aux_750]
    type = HenryGasConstantScalarAux
    T = 750 # K
    radius = 1.88e-10 # m argon
    salt = flibe
    variable = argon_henry_750
  []
  [argon_henry_aux_900]
    type = HenryGasConstantScalarAux
    T = 900 # K
    radius = 1.88e-10 # m argon
    salt = flibe
    variable = argon_henry_900
  []
  [argon_henry_aux_1100]
    type = HenryGasConstantScalarAux
    T = 1100 # K
    radius = 1.88e-10 # m argon
    salt = flibe
    variable = argon_henry_1100
  []
[]

# Nonlinear variable, BCs, and Kernels are not used for anything but needed to get AuxVariable solution
[Variables]
  [temperature]
  []
[]

[Kernels]
  [temperature]
    type = Diffusion
    variable = temperature
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = temperature
    value = 800
    boundary = left
  []
  [right]
    type = DirichletBC
    variable = temperature
    value = 800
    boundary = right
  []
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [helium_750]
    type = ScalarVariable
    variable = helium_henry_750
    outputs = none
  []
  # Note that the reference paper gives results in mol/cm3/atm, but MOOSE works in units of
  # mol/m3/Pa, so unit conversion is needed here
  [helium_gold_750]
    type = ConstantPostprocessor
    value = ${fparse 4.8e-8/(1e-6*101325)}
    outputs = none
  []
  # Set tolerance to second sig fig
  [check_helium_750]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = helium_750
    value_b = helium_gold_750
    absolute_tolerance = 0.1e-7
  []
  [helium_900]
    type = ScalarVariable
    variable = helium_henry_900
    outputs = none
  []
  [helium_gold_900]
    type = ConstantPostprocessor
    value = ${fparse 8.5e-8/(1e-6*101325)}
    outputs = none
  []
  [check_helium_900]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = helium_900
    value_b = helium_gold_900
    absolute_tolerance = 0.1e-7
  []
  [helium_1100]
    type = ScalarVariable
    variable = helium_henry_1100
    outputs = none
  []
  [helium_gold_1100]
    type = ConstantPostprocessor
    value = ${fparse 1.4e-7/(1e-6*101325)}
    outputs = none
  []
  [check_helium_1100]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = helium_1100
    value_b = helium_gold_1100
    absolute_tolerance = 0.1e-6
  []
  [argon_750]
    type = ScalarVariable
    variable = argon_henry_750
    outputs = none
  []
  [argon_gold_750]
    type = ConstantPostprocessor
    value = ${fparse 4.3e-9/(1e-6*101325)}
    outputs = none
  []
  # Set tolerance to second sig fig
  [check_argon_750]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = argon_750
    value_b = argon_gold_750
    absolute_tolerance = 0.1e-8
  []
  [argon_900]
    type = ScalarVariable
    variable = argon_henry_900
    outputs = none
  []
  [argon_gold_900]
    type = ConstantPostprocessor
    value = ${fparse 1.2e-8/(1e-6*101325)}
    outputs = none
  []
  [check_argon_900]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = argon_900
    value_b = argon_gold_900
    absolute_tolerance = 0.1e-7
  []
  [argon_1100]
    type = ScalarVariable
    variable = argon_henry_1100
    outputs = none
  []
  [argon_gold_1100]
    type = ConstantPostprocessor
    value = ${fparse 3.1e-8/(1e-6*101325)}
    outputs = none
  []
  [check_argon_1100]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = argon_1100
    value_b = argon_gold_1100
    absolute_tolerance = 0.1e-7
  []

[]

[Outputs]
  exodus = false
  [csv]
    type = CSV
    start_time = 1
  []
[]
