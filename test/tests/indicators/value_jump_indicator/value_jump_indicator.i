###########################################################
# This is a test of the Mesh Indicator System. It computes
# a user-defined "error" for each element in the Mesh.
#
# This test has been verified to give the same error
# calculation as the libMesh kelly_error_estimator.  If
# this test is diffing... the diff is wrong!
#
# @Requirement F2.40
###########################################################

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./something]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

# Mesh Indicator System
[Adaptivity]
  [./Indicators]
    [./error]
      type = ValueJumpIndicator
      variable = something
    [../]
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
