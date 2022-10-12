[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 2
    dx = '4 2 3'
    dy = '1 2'
    ix = '10 10 10'
    iy = '8 8'
    subdomain_id = '1 2 3
                    2 2 2'
  []

  [ed0]
    type = BlockDeletionGenerator
    input = cmg
    block = '1 3'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [dt]
    type = TimeDerivative
    variable = u
  []
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [top]
    type = DirichletBC
    variable = u
    boundary = bottom
    value = 1
  []
[]

[Executioner]
  type = Transient
  start_time = 0
  end_time = 10
  dt = 10

  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
