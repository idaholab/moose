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
#    ix = '10 10'
#    dy = '1'
#    iy = '10'
#    subdomain_id = '1 2'
#  [../]
#
#  [./interior_bc]
#    type = SideSetsBetweenSubdomainsGenerator
#    primary_block = 1
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

[Executioner]
  type = Steady
[]

[AuxVariables]
  [./id1]
    family = MONOMIAL
    order = CONSTANT
  []

  [./id2]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [./id1]
    type = BIDAux
    variable = id1
    boundary = 'top'
  [../]

  [./id2]
    type = BIDAux
    variable = id2
    boundary = '12'
  [../]
[]



[Outputs]
  exodus = true
[]
