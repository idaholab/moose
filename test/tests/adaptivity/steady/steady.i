[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [subdomain]
    type = SubdomainBoundingBoxGenerator
    input = gen
    bottom_left = '0.25 0.25 0'
    top_right = '0.75 0.75 0'
    block_id = 100
  []
[]

[Variables/u]
[]

[Kernels/diff]
  type = Diffusion
  variable = u
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Adaptivity]
  initial_marker = uniform
  initial_steps = 1
  [Markers/uniform]
    type = UniformMarker
    mark = REFINE
    block = 100
  []
[]

[Outputs]
  exodus = true
[]
