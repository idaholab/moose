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
    u = u
    diff = 'diff'
    ad_diff = 'ad_diff'
  []
[]

[Kernels]
  [diff]
    type = MatDiffusion
    variable = u
    diffusivity = 'diff'
  []
[]

[DGKernels]
  [dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
    diff = 'diff'
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
    diff = 'diff'
  []
[]

[Materials]
  [constant]
    type = GenericConstantMaterial
    prop_names = 'diff'
    prop_values = '2'
  []
  [ad_constant]
    type = ADGenericConstantMaterial
    prop_names = 'ad_diff'
    prop_values = '2'
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-14
[]

[Outputs]
  exodus = true
[]
