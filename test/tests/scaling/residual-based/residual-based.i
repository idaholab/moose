[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 20
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = '1000 * (1 - x)'
  []
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
  [rxn]
    type = PReaction
    power = 2
    variable = u
  []
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1000
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  verbose = true
  automatic_scaling = true
  resid_vs_jac_scaling_param = 1
[]

[Outputs]
  exodus = true
[]
