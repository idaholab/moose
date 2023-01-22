[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./u]
    order = FIRST
    family = MONOMIAL
  [../]
[]

[AuxVariables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    expression = 2*pow(e,-x-(y*y))*(1-2*y*y)
  [../]
  [./exact_fn]
    type = ParsedGradFunction
    expression = pow(e,-x-(y*y))
    grad_x = -pow(e,-x-(y*y))
    grad_y = -2*y*pow(e,-x-(y*y))
  [../]
  [./disp_func]
    type = ParsedFunction
    expression = x
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
  [./abs]
    type = Reaction
    variable = u
  [../]
  [./forcing]
    type = BodyForce
    variable = u
    function = forcing_fn
  [../]
[]

[DGKernels]
  [./dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
    use_displaced_mesh = true
  [../]
[]

[BCs]
  [./all]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
    epsilon = -1
    sigma = 6
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_rel_tol = 1e-10
[]

[Outputs]
  execute_on = 'timestep_end'
  file_base = out
  exodus = true
[]

[ICs]
  [./disp_x_ic]
    function = disp_func
    variable = disp_x
    type = FunctionIC
  [../]
[]

