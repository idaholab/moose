[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 80
    xmax = 4
  []
  [subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '2.0 0 0'
    block_id = 1
    top_right = '4.0 1.0 0'
  []
  [left_right]
    input = subdomain1
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'left_right'
  []
  [right_left]
    input = left_right
    type = SideSetsBetweenSubdomainsGenerator
    primary_block = '1'
    paired_block = '0'
    new_boundary = 'right_left'
  []
[]

[Variables]
  [left_fv]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
    block = 0
  []
  [left_fe]
    initial_condition = 1
    block = 0
  []
  [right_fv]
    family = MONOMIAL
    order = CONSTANT
    fv = true
    initial_condition = 1
    block = 1
  []
  [right_fe]
    initial_condition = 1
    block = 1
  []
[]

[FVKernels]
  active = 'bad_left_diff left_coupled bad_right_diff right_coupled'
  [bad_left_diff]
    type = FVDiffusion
    variable = left_fv
    coeff = fv_prop
    block = 0
    coeff_interp_method = average
  []
  [good_left_diff]
    type = FVDiffusion
    variable = left_fv
    coeff = left_fv_prop
    block = 0
    coeff_interp_method = average
  []
  [left_coupled]
    type = FVCoupledForce
    v = left_fv
    variable = left_fv
    block = 0
  []
  [bad_right_diff]
    type = FVDiffusion
    variable = right_fv
    coeff = fv_prop
    block = 1
    coeff_interp_method = average
  []
  [good_right_diff]
    type = FVDiffusion
    variable = right_fv
    coeff = right_fv_prop
    block = 1
    coeff_interp_method = average
  []
  [right_coupled]
    type = FVCoupledForce
    v = right_fv
    variable = right_fv
    block = 1
  []
[]

[Kernels]
  [left_diff]
    type = ADFunctorMatDiffusion
    variable = left_fe
    diffusivity = fe_prop
  []
  [left_coupled]
    type = CoupledForce
    v = left_fv
    variable = left_fe
  []
  [right_diff]
    type = ADFunctorMatDiffusion
    variable = right_fe
    diffusivity = fe_prop
  []
  [right_coupled]
    type = CoupledForce
    v = right_fv
    variable = right_fe
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = left_fv
    boundary = left
    value = 0
  []
  [left_right]
    type = FVDirichletBC
    variable = left_fv
    boundary = left_right
    value = 1
  []
  [right_left]
    type = FVDirichletBC
    variable = right_fv
    boundary = right_left
    value = 0
  []
  [right]
    type = FVDirichletBC
    variable = right_fv
    boundary = right
    value = 1
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = left_fe
    boundary = left
    value = 0
  []
  [left_right]
    type = DirichletBC
    variable = left_fe
    boundary = left_right
    value = 1
  []
  [right_left]
    type = DirichletBC
    variable = right_fe
    boundary = right_left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = right_fe
    boundary = right
    value = 1
  []
[]

[Materials]
  active = 'fe_mat_left bad_fv_mat_left fe_mat_right bad_fv_mat_right'
  [fe_mat_left]
    type = FEFVCouplingMaterial
    fe_var = left_fe
    block = 0
  []
  [bad_fv_mat_left]
    type = FEFVCouplingMaterial
    fv_var = left_fv
    block = 0
  []
  [good_fv_mat_left]
    type = FEFVCouplingMaterial
    fv_var = left_fv
    fv_prop_name = 'left_fv_prop'
    block = 0
  []
  [fe_mat_right]
    type = FEFVCouplingMaterial
    fe_var = right_fe
    block = 1
  []
  [bad_fv_mat_right]
    type = FEFVCouplingMaterial
    fv_var = right_fv
    block = 1
  []
  [good_fv_mat_right]
    type = FEFVCouplingMaterial
    fv_var = right_fv
    fv_prop_name = 'right_fv_prop'
    block = 1
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
