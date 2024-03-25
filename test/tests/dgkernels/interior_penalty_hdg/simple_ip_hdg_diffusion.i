[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  elem_type = QUAD9
[]

[Variables]
  [u]
    order = FIRST
    family = MONOMIAL
  []
  [side_u]
    order = FIRST
    family = SIDE_HIERARCHIC
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[DGKernels]
  [dg_interior]
    type = ADHDGDiffusion
    variable = u
    side_variable = side_u
    alpha = 6
  []
  [dg_side]
    type = ADHDGDiffusionSide
    variable = side_u
    interior_variable = u
    alpha = 6
  []
[]

[BCs]
  [left_interior]
    type = HDGDiffusionBC
    variable = u
    exact_soln = 0
    boundary = 'left'
    alpha = 6
  []
  [left_side]
    type = ADHDGSideDirichletBC
    variable = side_u
    exact_soln = 0
    boundary = 'left'
  []
  [right_interior]
    type = HDGDiffusionBC
    variable = u
    exact_soln = 1
    boundary = 'right'
    alpha = 6
  []
  [right_side]
    type = ADHDGSideDirichletBC
    variable = side_u
    exact_soln = 1
    boundary = 'right'
  []
  [zero_flux]
    type = HDGDiffusionFluxBC
    variable = u
    side_variable = side_u
    alpha = 6
    boundary = 'top bottom'
  []
  [zero_flux_side]
    type = HDGDiffusionFluxSideBC
    variable = side_u
    interior_variable = u
    alpha = 6
    boundary = 'top bottom'
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-10
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  dofmap = true
[]
