[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Postprocessors]
  [./L2_norm]
    type = ElementL2Norm
    variable = u
  [../]
  [L1_norm]
    type = ElementL1Error
    function = 0
    variable = u
  []
  [parsed]
    type = ParsedPostprocessor
    function = 'L2_norm / L1_norm'
    pp_names = 'L2_norm L1_norm'
  []
  [parsed_with_t]
    type = ParsedPostprocessor
    function = 'L2_norm + L1_norm + t'
    pp_names = 'L2_norm L1_norm'
    use_t = true
  []
  [parsed_with_constants]
    type = ParsedPostprocessor
    function = 'L2_norm + 3*L1_norm + mu'
    pp_names = 'L2_norm L1_norm'
    constant_names = 'mu'
    constant_expressions = '4'
  []
[]

[Executioner]
  type = Transient
  num_steps = 4
  nl_abs_tol = 1e-8
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  line_search = 'none'
[]

[Outputs]
  csv = true
[]
