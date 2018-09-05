###########################################################
# This is a simple test of the Material System. A
# user-defined Material (MTMaterial) is providing a
# Real property named "matp" that varies spatially
# throughout the domain. This property is used as a
# coefficient by MatDiffusionTest. It is also output
# by MaterialRealAux for visualization purposes.
#
# @Requirement F4.10
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

[AuxVariables]
  [./mat]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = MatDiffusionTest
    variable = u
    prop_name = matp
  [../]
[]

[AuxKernels]
  [./mat]
    type = MaterialRealAux
    variable = mat
    property = matp
    execute_on = timestep_end
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 1
  [../]

  [./right]
    type = MTBC
    variable = u
    boundary = 1
    grad = 8
    prop_name = matp
  [../]
[]

# Materials System
[Materials]
  [./mat]
    type = MTMaterial
    block = 0
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
[]
