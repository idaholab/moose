[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[ICs]
  [u]
    type = FunctionIC
    variable = u
    function = '1000 * (1 - x)'
  []
  [v]
    type = FunctionIC
    variable = v
    function = '1e-3 * (1 - x)'
  []
[]

[Variables]
  [u][]
  [v][]
[]

[Problem]
  type = ReferenceResidualProblem
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    extra_vector_tags = 'ref'
  [../]
  [rxn]
    type = PReaction
    power = 2
    variable = u
    extra_vector_tags = 'ref'
  []
  [./diff_v]
    type = Diffusion
    variable = v
    extra_vector_tags = 'ref'
  [../]
  [rxn_v]
    type = PReaction
    power = 2
    variable = v
    extra_vector_tags = 'ref'
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
  [./left_v]
    type = DirichletBC
    variable = v
    boundary = left
    value = 1e-3
  [../]
  [./right_v]
    type = DirichletBC
    variable = v
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
