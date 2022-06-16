[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  elem_type = QUAD4
[]

[Variables]
  [u]
    order = FIRST
    family = MONOMIAL
  []
[]

[Functions]
  [exact_fn]
    type = ParsedGradFunction
    value = pow(e,-x-(y*y))
    grad_x = -pow(e,-x-(y*y))
    grad_y = -2*y*pow(e,-x-(y*y))
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
    diff = diffusion
  []
[]

[Materials]
  [coupled_mat]
    type = VarCouplingMaterial
    var = u
    declare_old = true
    use_tag = false
  []
[]

[BCs]
  [all]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
    epsilon = -1
    sigma = 6
  []
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
[]
