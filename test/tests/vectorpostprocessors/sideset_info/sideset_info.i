[Mesh]
  type = MeshGeneratorMesh

  [./uniform]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    nx = 8
    ymin = -0.4
    ymax = 10.4
    ny = 5
  [../]
[]

[Variables]
  [./u]
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

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[VectorPostprocessors]
  [./side_info]
    type = SidesetInfoVectorPostprocessor
    boundary = 'left right bottom'
    meta_data_types = 'centroid min max area'
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  file_base = out
  csv = true
[]
