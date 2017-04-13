[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[Variables]
  [./u]
  [../]
[]

[Functions]
  [./ffn]
    type = ParsedFunction
    value = '2 - t'
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./ffn]
    type = UserForcingFunction
    variable = u
    function = ffn
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 1
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 0
  [../]
[]

[Postprocessors]
  [./rsn]
    type = RelativeSolutionDifferenceNorm
    execute_on = TIMESTEP_END
  [../]
[]

[Executioner]
  type = Transient
  dt = 1
  num_steps = 2
  trans_ss_check = true
  ss_check_tol = 1e-08
[]

[Outputs]
  exodus = true
[]
