[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    xmax = 1.5
    dim = 1
    nx = 3
  []
  [sub1]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    block_id = 1
    input = gen
  []
  [sub2]
    type = SubdomainBoundingBoxGenerator
    bottom_left = '1 0 0'
    top_right = '1.5 1 0'
    block_id = 2
    input = sub1
  []
  displacements = 'disp_x'
[]

[Variables]
  [u][]
[]

[AuxVariables]
  [disp_x]
    block = 1
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
    use_displaced_mesh = true
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    value = 0
    boundary = left
  []
  [right]
    type = DirichletBC
    variable = u
    value = 1
    boundary = right
  []
[]

[Executioner]
  type = Steady
[]
