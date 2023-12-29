[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  elem_type = QUAD9
  uniform_refine = 0
[]

[Variables]
  [disp_x]
    order = SECOND
    family = LAGRANGE
  []
  [disp_y]
    order = SECOND
    family = LAGRANGE
  []
[]

[AuxVariables]
  [stress_xx]
    order = FIRST
    family = MONOMIAL
  []
  [stress_yy]
    order = FIRST
    family = MONOMIAL
  []
  [stress_xx_recovered]
    order = SECOND
    family = LAGRANGE
  []
  [stress_yy_recovered]
    order = SECOND
    family = LAGRANGE
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = 'timestep_end'
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = 'timestep_end'
  []
  [stress_xx_recovered]
    type = NodalPatchRecoveryAux
    variable = stress_xx_recovered
    nodal_patch_recovery_uo = stress_xx_patch
    execute_on = 'TIMESTEP_END'
  []
  [stress_yy_recovered]
    type = NodalPatchRecoveryAux
    variable = stress_yy_recovered
    nodal_patch_recovery_uo = stress_yy_patch
    execute_on = 'TIMESTEP_END'
  []
[]

[Kernels]
  [solid_x]
    type = StressDivergenceTensors
    variable = disp_x
    component = 0
  []
  [solid_y]
    type = StressDivergenceTensors
    variable = disp_y
    component = 1
  []
[]

[Materials]
  [strain]
    type = ComputeSmallStrain
  []
  [Cijkl]
    type = ComputeIsotropicElasticityTensor
    poissons_ratio = 0.3
    youngs_modulus = 2.1e+5
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[BCs]
  [top_xdisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top'
    function = 0
  []
  [top_ydisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'top'
    function = t
  []
  [bottom_xdisp]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 'bottom'
    function = 0
  []
  [bottom_ydisp]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 'bottom'
    function = 0
  []
[]

[UserObjects]
  [stress_xx_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = SECOND
    property = 'stress'
    component = '0 0'
    execute_on = 'TIMESTEP_END'
  []
  [stress_yy_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = SECOND
    property = 'stress'
    component = '1 1'
    execute_on = 'TIMESTEP_END'
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
    ksp_norm = default
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-ksp_type -pc_type'
  petsc_options_value = 'preonly   lu'
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-8
  l_max_its = 100
  nl_max_its = 30
  dt = 0.01
  dtmin = 1e-11
  start_time = 0
  end_time = 0.05
[]

[Outputs]
  interval = 1
  exodus = true
  print_linear_residuals = false
[]
