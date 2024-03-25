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
  [side_u]
    order = FIRST
    family = SIDE_HIERARCHIC
  []
[]

[Kernels]
  [adv]
    type = ADConservativeAdvection
    variable = u
    velocity = vel
  []
[]

[DGKernels]
  [dg_interior_adv]
    type = ADHDGAdvection
    variable = u
    side_variable = side_u
    velocity = vel
  []
  [dg_side_adv]
    type = ADHDGAdvectionSide
    variable = side_u
    interior_variable = u
    velocity = vel
  []
[]

[BCs]
  [dg_interior_adv]
    type = ADHDGAdvectionDirichletBC
    variable = u
    exact_soln = 1
    velocity = vel
    boundary = 'left right'
  []
  [dirichlet_side]
    type = ADHDGSideDirichletBC
    variable = side_u
    exact_soln = 1
    boundary = 'left right'
  []
[]

[Materials]
  [vel]
    type = ADGenericConstantVectorMaterial
    prop_names = 'vel'
    prop_values = '2 0 0'
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
