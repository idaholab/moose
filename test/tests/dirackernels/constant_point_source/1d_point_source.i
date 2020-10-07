###########################################################
# This is test of the Dirac delta function System. The
# ConstantPointSource object is used to apply a constant
# Dirac delta contribution at a specified point in the
# domain.
#
# @Requirement F3.50
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables]
  active = 'u'
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[DiracKernels]
  [./point_source1]
    type = ConstantPointSource
    variable = u
    value = 1.0
    point = '0.2 0 0'
  [../]
  [./point_source2]
    type = ConstantPointSource
    variable = u
    value = -0.5
    point = '0.7 0 0'
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

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

[]

[Outputs]
  file_base = 1d_out
  exodus = true
[]
