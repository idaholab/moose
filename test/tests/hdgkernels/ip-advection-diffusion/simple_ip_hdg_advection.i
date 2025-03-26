[GlobalParams]
  variable = u
  face_variable = side_u
  velocity = vel
[]

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

[HDGKernels]
  [adv]
    type = AdvectionIPHDGKernel
  []
[]

[BCs]
  [inflow]
    type = AdvectionIPHDGDirichletBC
    functor = 1
    boundary = 'left'
  []
  [outflow]
    type = AdvectionIPHDGOutflowBC
    boundary = 'right'
  []
[]

[FunctorMaterials]
  [vel]
    type = ADGenericVectorFunctorMaterial
    prop_names = 'vel'
    prop_values = '2 0 0'
  []
[]

[Executioner]
  type = Steady
  nl_rel_tol = 1e-10
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu       NONZERO'
  solve_type = NEWTON
[]

[Postprocessors]
  [symmetric]
    type = IsMatrixSymmetric
  []
  [avg_u]
    type = ElementAverageValue
    variable = u
  []
  [avg_side_u]
    type = ElementAverageValue
    variable = side_u
  []
[]

[Outputs]
  csv = true
[]
