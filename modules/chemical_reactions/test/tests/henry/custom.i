# Test of HenryGasConstantAux for different temperatures
# This test is for a custom defined model
#
# Ensure that all "check_" values evaluate to 1

helium_radius    = 1.4e-10 # m
helium_alpha     = 3.0
helium_beta      = 0.01
helium_gamma_0   = 235.0
helium_dgamma_dT = -0.08
helium_KH0       = 8e-7

argon_radius    = 1.88e-10 # m
argon_alpha     = 3.0
argon_beta      = 0.01
argon_gamma_0   = 235.0
argon_dgamma_dT = -0.08
argon_KH0       = 8e-7

m_to_ang = 1e10

Rgas = 8.3144626

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  xmin = 0
  xmax = 1
[]

[AuxVariables]
  [helium_henry_750]
  []
  [helium_henry_900]
  []
  [helium_henry_1100]
  []
  [argon_henry_750]
  []
  [argon_henry_900]
  []
  [argon_henry_1100]
  []
[]

[AuxKernels]
  [helium_henry_aux_750]
    type = HenryGasConstantAux
    T = 750 # K
    radius = ${helium_radius} # m helium
    salt = custom
    variable = helium_henry_750
    alpha     = ${helium_alpha}
    beta      = ${helium_beta}
    gamma_0   = ${helium_gamma_0}
    dgamma_dT = ${helium_dgamma_dT}
    KH0       = ${helium_KH0}
  []
  [helium_henry_aux_900]
    type = HenryGasConstantAux
    T = 900 # K
    radius = ${helium_radius} # m helium
    salt = custom
    variable = helium_henry_900
    alpha     = ${helium_alpha}
    beta      = ${helium_beta}
    gamma_0   = ${helium_gamma_0}
    dgamma_dT = ${helium_dgamma_dT}
    KH0       = ${helium_KH0}
  []
  [helium_henry_aux_1100]
    type = HenryGasConstantAux
    T = 1100 # K
    radius = ${helium_radius} # m helium
    salt = custom
    variable = helium_henry_1100
    alpha     = ${helium_alpha}
    beta      = ${helium_beta}
    gamma_0   = ${helium_gamma_0}
    dgamma_dT = ${helium_dgamma_dT}
    KH0       = ${helium_KH0}
  []
  [argon_henry_aux_750]
    type = HenryGasConstantAux
    T = 750 # K
    radius = ${argon_radius} # m argon
    salt = custom
    variable = argon_henry_750
    alpha     = ${argon_alpha}
    beta      = ${argon_beta}
    gamma_0   = ${argon_gamma_0}
    dgamma_dT = ${argon_dgamma_dT}
    KH0       = ${argon_KH0}
  []
  [argon_henry_aux_900]
    type = HenryGasConstantAux
    T = 900 # K
    radius = ${argon_radius}
    salt = custom
    variable = argon_henry_900
    alpha     = ${argon_alpha}
    beta      = ${argon_beta}
    gamma_0   = ${argon_gamma_0}
    dgamma_dT = ${argon_dgamma_dT}
    KH0       = ${argon_KH0}
  []
  [argon_henry_aux_1100]
    type = HenryGasConstantAux
    T = 1100 # K
    radius = ${argon_radius}
    salt = custom
    variable = argon_henry_1100
    alpha     = ${argon_alpha}
    beta      = ${argon_beta}
    gamma_0   = ${argon_gamma_0}
    dgamma_dT = ${argon_dgamma_dT}
    KH0       = ${argon_KH0}
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
  [helium_henry_750]
    type = PointValue
    variable = helium_henry_750
    point = '1 0 0'
    outputs = none
  []
  # Note that the reference paper gives results in mol/cm3/atm, but MOOSE works in units of
  # mol/m3/Pa, so unit conversion is needed here
  [helium_gold_750]
    type = ConstantPostprocessor
    value = ${fparse exp(
       (helium_alpha * 4*pi*pow(helium_radius*m_to_ang, 2)*(helium_gamma_0+helium_dgamma_dT*(750.0-273.15)) +
        helium_beta * 4.0/3.0*pi*pow(helium_radius*m_to_ang, 3)*Rgas*750.0)/
       (Rgas*750.0)
       )*helium_KH0}
    outputs = none
  []
  [check_helium_750]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = helium_henry_750
    value_b = helium_gold_750
    absolute_tolerance = 0.1e-7
  []
  [helium_henry_900]
    type = PointValue
    variable = helium_henry_900
    point = '1 0 0'
    outputs = none
  []
  [helium_gold_900]
    type = ConstantPostprocessor
    value = ${fparse exp(
       (helium_alpha * 4*pi*pow(helium_radius*m_to_ang, 2)*(helium_gamma_0+helium_dgamma_dT*(900.0-273.15)) +
        helium_beta * 4.0/3.0*pi*pow(helium_radius*m_to_ang, 3)*Rgas*900.0)/
       (Rgas*900.0)
       )*helium_KH0}

    outputs = none
  []
  [check_helium_900]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = helium_henry_900
    value_b = helium_gold_900
    absolute_tolerance = 0.1e-7
  []
  [helium_henry_1100]
    type = PointValue
    variable = helium_henry_1100
    point = '1 0 0'
    outputs = none
  []
  [helium_gold_1100]
    type = ConstantPostprocessor
    value = ${fparse exp(
       (helium_alpha * 4*pi*pow(helium_radius*m_to_ang, 2)*(helium_gamma_0+helium_dgamma_dT*(1100.0-273.15)) +
        helium_beta * 4.0/3.0*pi*pow(helium_radius*m_to_ang, 3)*Rgas*1100.0)/
       (Rgas*1100.0)
       )*helium_KH0}
    outputs = none
  []
  [check_helium_1100]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = helium_henry_1100
    value_b = helium_gold_1100
    absolute_tolerance = 0.1e-6
  []
  [argon_henry_750]
    type = PointValue
    variable = argon_henry_750
    point = '1 0 0'
    outputs = none
  []
  [argon_gold_750]
    type = ConstantPostprocessor
    value = ${fparse exp(
       (argon_alpha * 4*pi*pow(argon_radius*m_to_ang, 2)*(argon_gamma_0+argon_dgamma_dT*(750.0-273.15)) +
        argon_beta * 4.0/3.0*pi*pow(argon_radius*m_to_ang, 3)*Rgas*750.0)/
       (Rgas*750.0)
       )*argon_KH0}
    outputs = none
  []
  # Set tolerance to second sig fig
  [check_argon_750]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = argon_henry_750
    value_b = argon_gold_750
    absolute_tolerance = 0.1e-8
  []
  [argon_henry_900]
    type = PointValue
    variable = argon_henry_900
    point = '1 0 0'
    outputs = none
  []
  [argon_gold_900]
    type = ConstantPostprocessor
    value = ${fparse exp(
       (argon_alpha * 4*pi*pow(argon_radius*m_to_ang, 2)*(argon_gamma_0+argon_dgamma_dT*(900.0-273.15)) +
        argon_beta * 4.0/3.0*pi*pow(argon_radius*m_to_ang, 3)*Rgas*900.0)/
       (Rgas*900.0)
       )*argon_KH0}
    outputs = none
  []
  [check_argon_900]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = argon_henry_900
    value_b = argon_gold_900
    absolute_tolerance = 0.1e-7
  []
  [argon_henry_1100]
    type = PointValue
    variable = argon_henry_1100
    point = '1 0 0'
    outputs = none
  []
  [argon_gold_1100]
    type = ConstantPostprocessor
    value = ${fparse exp(
       (argon_alpha * 4*pi*pow(argon_radius*m_to_ang, 2)*(argon_gamma_0+argon_dgamma_dT*(1100.0-273.15)) +
        argon_beta * 4.0/3.0*pi*pow(argon_radius*m_to_ang, 3)*Rgas*1100.0)/
       (Rgas*1100.0)
       )*argon_KH0}
    outputs = none
  []
  [check_argon_1100]
    type = PostprocessorComparison
    comparison_type = 'equals'
    value_a = argon_henry_1100
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
