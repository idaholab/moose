###########################################################
# This is a test of the Geometric Search System. This test
# uses the nearest node locator through the
# NearestNodeDistanceAux Auxilary Kernel to record the
# distance to the nearest nodes along paired
# boundaries.
#
# @Requirement F6.50
###########################################################

[Mesh]
  file = 2dcontact_collide.e
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./distance]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./distance]
    type = NearestNodeDistanceAux
    variable = distance
    boundary = 2
    paired_boundary = 3
  [../]
[]

[BCs]
  [./block1_left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./block1_right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
  [./block2_left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]
  [./block2_right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

[]

[Outputs]
  exodus = true
   [./pgraph]
    type = PerfGraphOutput
    heaviest_branch = true
    heaviest_sections = 5
    level = 2
  [../]
[]
