###########################################################
# This is a test of the Geometric Search System. This test
# uses the penetration location object through the
# PenetrationAux Auxilary Kernel to detect overlaps of
# specified interfaces (boundaries) in the domain.
#
# @Requirement F6.50
###########################################################


[Mesh]
  [connected_mesh]
    type = FileMeshGenerator
    file = 2dcontact_collide.e
  []
  [exploded_mesh]
    type = BreakMeshByElementGenerator
    input = connected_mesh
    subdomains = '1 2'
    interface_name = 'interelement'
  []
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./penetration]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./l2]
    type = MassMatrix
    variable = u
    matrix_tags = 'system'
  [../]
[]

[AuxKernels]
  [./penetrate]
    type = PenetrationAux
    variable = penetration
    boundary = 2
    paired_boundary = 3
    search_using_point_locator = true
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
