#
# This is used to create the mesh but it does not work with --distributed-mesh flag
# and the parallel test bombs.
#
#[Mesh]
#  type = MeshGeneratorMesh
#
#  [./cartesian]
#    type = CartesianMeshGenerator
#    dim = 2
#    dx = '1 1'
#    dy = '1'
#    subdomain_id = '1 2'
#  [../]
#
#  [./interior_bc]
#    type = SideSetsBetweenSubdomainsGenerator
#    master_block = 1
#    paired_block = 2
#    new_boundary = 12
#    input = cartesian
#  [../]
#[]

[Mesh]
  type = FileMesh
  file = current_boundary_id_in.e
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./u]
  [../]
[]

[Postprocessors]
  [./interior_boundary]
    type = AverageBID
    boundary = 12
  [../]

  [./top_boundary]
    type = AverageBID
    boundary = top
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
