# Steady-state test for the ThermoDiffusion kernel.
#
# This test applies a constant temperature gradient to drive thermo-diffusion
# in the variable u. At steady state, the thermo-diffusion is balanced by
# diffusion due to Fick's Law, so the total flux is
#
#   J = -D ( grad(u) - ( Qstar u / R ) grad(1/T) )
#
# If there are no fluxes at the boundaries, then there is no background flux and
# these two terms must balance each other everywhere:
#
#   grad(u) = ( Qstar u / R ) grad(1/T)
#
# The dx can be eliminated to give
#
#   d(ln u) / d(1/T) = Qstar / R
#
# This can be solved to give the profile for u as a function of temperature:
#
#   u = A exp( Qstar / R T )
#
# Here, we are using simple heat conduction with Dirichlet boundaries on 0 <= x <= 1
# to give a linear profile for temperature: T = x + 1. We also need to apply one
# boundary condition on u, which is u(x=0) = 1. These conditions give:
#
#   u = exp( -(Qstar/R) (x/(x+1)) )
#
# This analytical result is tracked by the aux variable "correct_u".

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
[]

[Variables]
  [./u]
    initial_condition = 1
  [../]
  [./temp]
    initial_condition = 1
  [../]
[]

[Kernels]
  [./soret]
    type = ADThermoDiffusion
    variable = u
    temperature = temp
  [../]
  [./diffC]
    type = ADDiffusion
    variable = u
  [../]

  # Heat diffusion gives a linear temperature profile to drive the Soret diffusion.
  [./diffT]
    type = ADDiffusion
    variable = temp
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    preset = false
    boundary = left
    value = 1
  [../]

  [./leftt]
    type = DirichletBC
    variable = temp
    preset = false
    boundary = left
    value = 1
  [../]
  [./rightt]
    type = DirichletBC
    variable = temp
    preset = false
    boundary = right
    value = 2
  [../]
[]

[Materials]
  [./ad_soret_coefficient]
    type = ADSoretCoeffTest
    temperature = temp
    coupled_var = u
  [../]
[]

[Preconditioning]
  [./full]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [./error]
    type = NodalL2Error
    variable = u
    function = 'exp(-x/(x+1))'
  [../]
[]

[Outputs]
  execute_on = FINAL
  exodus = true
[]
