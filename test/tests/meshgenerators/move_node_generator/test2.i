[Mesh]
  allow_renumbering=false
  inactive = 'missingNode mismatchSize'
  [./eg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1'
    dy = '1'
    dz = '1'
    ix = '4'
    iy = '4'
    iz = '4'
    subdomain_id = '0'
  []
  [modifyNode]
    type = MoveNodeGenerator
    input = eg
    node_id = '0 1 2'
    new_position = '0.1 0 0
                    0.35 0 0
                    0.6 0 0'
  []
  [missingNode]
    type = MoveNodeGenerator
    input = eg
    node_id = '999'
    new_position = '0.1 0 0'
  []
  [mismatchSize]
    type = MoveNodeGenerator
    input = eg
    node_id = '0 1 2'
    new_position = '0.1 0 0'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
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

[Outputs]
  exodus = true
[]
