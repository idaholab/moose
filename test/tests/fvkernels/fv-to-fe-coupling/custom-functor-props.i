[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 40
    xmax = 2
  []
[]

[Variables]
  [fv]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
  []
  [fe]
    initial_condition = 1
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = fv
    coeff = fv_prop
  []
  [coupled]
    type = FVCoupledForce
    v = fv
    variable = fv
  []
[]

[Kernels]
  [diff]
    type = ADFunctorMatDiffusion
    variable = fe
    diffusivity = fe_prop
  []
  [coupled]
    type = CoupledForce
    v = fv
    variable = fe
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = fv
    boundary = left
    value = 0
  []
  [right]
    type = FVDirichletBC
    variable = fv
    boundary = right
    value = 1
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = fe
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = fe
    boundary = right
    value = 1
  []
[]

[Materials]
  [fe_mat]
    type = IMakeMyOwnFunctorProps
    fe_var = fe
  []
  [fv_mat]
    type = IMakeMyOwnFunctorProps
    fv_var = fv
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  line_search = 'none'
[]

[Outputs]
  exodus = true
[]
