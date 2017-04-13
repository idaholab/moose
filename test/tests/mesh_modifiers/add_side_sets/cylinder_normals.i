###########################################################
# This is test of the Mesh Modification System. This
# test adds sidesets to a mesh based on geometric normals
# to the faces or edges of the mesh. In this case sidesets
# are added to the top, bottom and side of a cylinder.
#
# @Requirement F2.20
###########################################################


[Mesh]
  type = FileMesh
  file = cylinder.e
  # This MeshModifier currently only works with ReplicatedMesh.
  # For more information, refer to #2129.
  parallel_type = replicated
[]

# Mesh Modifiers
[MeshModifiers]
  [./add_side_sets]
    type = SideSetsFromNormals
    normals = '0  0  1
               0  1  0
               0  0 -1'

    # This parameter allows the normal
    # to vary slightly from adjacent element
    # to element so that a sidset can follow
    # a curve. It is false by default.
    fixed_normal = false
    new_boundary = 'top side bottom'
  [../]
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
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 0
  [../]
  [./top]
    type = DirichletBC
    variable = u
    boundary = top
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
