[XFEM]
  qrule = volfrac
  output_cut_plane = true
[]

[UserObjects]
  [cut1]
    type = LevelSetCutUserObject
    level_set_var = ls1
    heal_always = true
  []
  [cut2]
    type = LevelSetCutUserObject
    level_set_var = ls2
    heal_always = true
  []
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
  xmax = 5
  ymax = 5
  elem_type = QUAD4
[]

[AuxVariables]
  [ls1]
  []
  [ls2]
  []
[]

[AuxKernels]
  [ls1]
    type = FunctionAux
    variable = ls1
    function = 'x-1.5'
  []
  [ls2]
    type = FunctionAux
    variable = ls2
    function = 'x-3.5'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = ADMatDiffusion
    variable = u
    diffusivity = diffusion_coefficient
  []
[]

[Constraints]
  [constraint1]
    type = XFEMSingleVariableConstraint
    variable = u
    use_displaced_mesh = false
    use_penalty = true
    alpha = 1e5
    geometric_cut_userobject = 'cut1'
  []
  [constraint2]
    type = XFEMSingleVariableConstraint
    variable = u
    use_displaced_mesh = false
    use_penalty = true
    alpha = 1e5
    geometric_cut_userobject = 'cut2'
  []
[]

[BCs]
  [right_u]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [left_u]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Materials]
  [diffusivity_A]
    type = ADGenericConstantMaterial
    prop_names = A_diffusion_coefficient
    prop_values = 1
  []
  [diffusivity_B]
    type = ADGenericConstantMaterial
    prop_names = B_diffusion_coefficient
    prop_values = 2
  []
  [diffusivity_C]
    type = ADGenericConstantMaterial
    prop_names = C_diffusion_coefficient
    prop_values = 3
  []

  [diff_combined]
    type = ADLevelSetMultiRealMaterial
    level_set_vars = 'ls1 ls2'
    base_name_keys = '-- +- ++'
    base_name_vals = 'A  B  C'
    prop_name = diffusion_coefficient
    outputs = exodus
  []
[]

[Executioner]
  type = Transient

  solve_type = 'NEWTON'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'

  automatic_scaling = true

  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-14

  num_steps = 1

  max_xfem_update = 1

  abort_on_solve_fail = true
[]

[Outputs]
  exodus = true
  csv = true
  file_base = glued_multimaterials_2d_out
[]
