[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 3
[]

[Variables]
  [u][]
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = FunctionDirichletBC
    variable = u
    function = '2 * t'
    boundary = left
  []
  [right]
    type = DirichletBC
    variable = u
    value = 0
    boundary = right
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  automatic_scaling = true
  compute_scaling_once = false
  solve_type = NEWTON
  resid_vs_jac_scaling_param = 1 # Pure residual scaling
  verbose = true
[]

[Outputs]
  exodus = true
[]
