[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
  # Ray tracing code is not yet compatible with DistributedMesh
  parallel_type = replicated
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

[VectorPostprocessors]
  [./intersections]
    type = IntersectionPointsAlongLine
    start = '0.05 0 0'
    end = '0.405 0 0'
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
[]
