[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Variables]
  [u]
    order = FIRST
    family = MONOMIAL
  []
[]

[UserObjects]
  [test]
    type = DGDiffusionDomainUserObject
    function = 'x'
    epsilon = -1
    sigma = 6
    execute_on = 'timestep_end'
    u = u
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[DGKernels]
  [dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
  []
[]

[BCs]
  [all]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = 'left right'
    function = 'x'
    epsilon = -1
    sigma = 6
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-14
[]

[Outputs]
  exodus = true
[]
