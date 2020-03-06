[Mesh]
  type = GeneratedMesh
  nx = 10
  xmax = 1
  ny = 10
  ymax = 1
  dim = 2
  allow_renumbering = false
[]

[Problem]
  type = FEProblem
  coord_type = RZ
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  [./out]
    type = Exodus
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
[]

[DGKernels]
  [./dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = MONOMIAL
  [../]
[]

[BCs]
  [./source]
    type = DGFunctionDiffusionDirichletBC
    variable = u
    boundary = 'right'
    function = exact_fn
    epsilon = -1
    sigma = 6
  [../]
  [./vacuum]
    boundary = 'top'
    type = VacuumBC
    variable = u
  [../]
[]

[Functions]
  [./exact_fn]
    type = ConstantFunction
    value = 1
  [../]
[]

[ICs]
  [./u]
    type = ConstantIC
    value = 1
    variable = u
  [../]
[]
