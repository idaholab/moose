[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 40
    xmax = 2
  []
[]

[Debug]
  show_functors = true
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
    coeff_interp_method = average
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
  active = 'fe_mat fv_mat'
  [bad_mat]
    type = FEFVCouplingMaterial
    fe_var = fe
    fv_var = fv
    execute_on = 'linear nonlinear'
  []
  [fe_mat]
    type = FEFVCouplingMaterial
    fe_var = fe
    execute_on = 'linear nonlinear'
  []
  [fv_mat]
    type = FEFVCouplingMaterial
    fv_var = fv
  []
  [fe_mat_bad_dep]
    type = FEFVCouplingMaterial
    fe_var = fe
    declared_prop_name = bad
  []
  [fv_mat_bad_dep]
    type = FEFVCouplingMaterial
    fv_var = fv
    retrieved_prop_name = bad
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
