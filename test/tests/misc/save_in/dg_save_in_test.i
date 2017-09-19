[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 9
  ny = 9
  elem_type = QUAD4
[]

[Variables]
  [./u]
    order = FIRST
    family = MONOMIAL

    [./InitialCondition]
      type = ConstantIC
      value = 1
    [../]
  [../]
[]

[AuxVariables]
  [./tot_resid]
    order = FIRST
    family = MONOMIAL
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
    save_in = 'tot_resid'
  [../]

  [./forcing]
    type = BodyForce
    variable = u
    function = 1
    save_in = 'tot_resid'
  [../]
[]

[DGKernels]
  [./dg_diff]
    type = DGDiffusion
    variable = u
    epsilon = -1
    sigma = 6
    save_in = 'tot_resid'
  [../]
[]

[BCs]
  [./robin]
    type = RobinBC
    boundary = 'left right top bottom'
    variable = u
    save_in = 'tot_resid'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  nl_rel_tol = 1e-10
[]

[Outputs]
  exodus = true
[]
