[Mesh]
[cmg]
  type = CartesianMeshGenerator
  dim = 2
  dx = '1 1 1 1'
  dy = '1 1 1 1'
  subdomain_id = '1 2 2 1
                  2 2 2 2
                  2 2 2 2
                  1 2 2 1'
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

  [src]
    type = CoupledForce
    variable = u
    v = -1000
    block = 1
  []
[]

[BCs]
  [left_bottom]
    type = DirichletBC
    variable = u
    boundary = 'left bottom'
    value = 0.0
  []
[]

[Postprocessors]
  [cost]
    type = PointValue
    variable = u
    point = '0.5 0.5 0'
  []
[]

[Executioner]
  type = Steady
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  print_linear_residuals = false
  print_nonlinear_residuals = false
  exodus = true
[]
