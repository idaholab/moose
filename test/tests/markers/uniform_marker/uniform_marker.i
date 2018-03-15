###########################################################
# This is a test of the Mesh Marker System. It marks
# elements with flags indicating whether they should be
# refined, coarsened, or left alone. This system
# has the ability to use the Mesh Indicator System.
#
# @Requirement F2.50
###########################################################


[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
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

[Executioner]
  type = Steady

  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

# Mesh Marker System
[Adaptivity]
  [./Markers]
    [./uniform]
      type = UniformMarker
      mark = refine
    [../]
  [../]
[]

[Outputs]
  exodus = true
[]
