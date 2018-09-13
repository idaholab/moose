###########################################################
# This is a simple test of the Material System. A
# user-defined Material (MTMaterial) is providing a
# Real property named "matp" that varies spatially
# throughout the domain. This property is used as a
# coefficient by MatDiffusionTest.
#
# This test verifies that when a material is restricted
# to a boundary, that MOOSE correctly reports that the
# volumetric material is missing.
###########################################################

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 4
  ny = 4
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusionTest
    variable = u
    prop_name = matp
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 'left'
    value = 1
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 'right'
    value = 2
  [../]
[]

# Materials System
[Materials]
  [./mat]
    type = MTMaterial
    boundary = 'bottom'
  [../]
[]

[Executioner]
  type = Steady
[]

# No output, this test is designed to produce an error
