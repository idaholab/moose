###########################################################
# This test exercises the parallel computation aspect of
# the framework. A Centroid partitioner is used to split
# the mesh into chunks for several processors along a
# vector (y-axis).
#
# @Requirement F2.30
###########################################################


[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2

    nx = 10
    ny = 100

    xmin = 0.0
    xmax = 1.0

    ymin = 0.0
    ymax = 10.0
  []

  # The centroid partitioner orders elements based on
  # the position of their centroids
  partitioner = centroid

  # This will order the elements based on the y value of
  # their centroid.  Perfect for meshes predominantly in
  # one direction
  centroid_partitioner_direction = y

  # The centroid partitioner behaves differently depending on
  # whether you are using Serial or DistributedMesh, so to get
  # repeatable results, we restrict this test to using ReplicatedMesh.
  parallel_type = replicated
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./proc_id]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[AuxKernels]
  [./proc_id]
    type = ProcessorIDAux
    variable = proc_id
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
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
