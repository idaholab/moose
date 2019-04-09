[Mesh]
  type = FileMesh
  file = 'quarter.e'
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialRankTwoTensorAux
    variable = stress_xx
    i = 0
    j = 0
    property = stress
  [../]
  [./stress_xy]
    type = MaterialRankTwoTensorAux
    variable = stress_xy
    i = 0
    j = 1
    property = stress
  [../]
  [./stress_yy]
    type = MaterialRankTwoTensorAux
    variable = stress_yy
    i = 1
    j = 1
    property = stress
  [../]
[]

[Kernels]
  [./stress_x]
    type = ADStressDivergenceTensors
    component = 0
    variable = disp_x
  [../]
  [./stress_y]
    type = ADStressDivergenceTensors
    component = 1
    variable = disp_y
  [../]
  [./ad_pf]
    type = ADPhaseFieldFracture
    l_name = l
    gc = gc_prop
    variable = c
  [../]
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
  [./c]
  [../]
[]

[Materials]
  [./pfbulkmat]
    type = GenericConstantMaterial
    prop_names = 'gc_prop l'
    prop_values = '1.0e-3 0.02'
  [../]
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '1.105e4 0.4540'
    fill_method = symmetric_isotropic_E_nu
  [../]
  [./strain]
    type = ADComputeGreenLagrangeStrain
  [../]
  [./elastic]
    type = ADComputeHyperElastoPlasticPFFractureStress
    yield_stress = 0.5
    linear_hardening_coefficient = 0
    c = c
  [../]
  # [./elastic]
  #   type = ADComputeHyperElasticPFFractureStress
  #   c = c
  # [../]
[]

[BCs]
  [./xdisp_inner]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = '(75.10/15 * t)* x / 10'
  [../]
  [./ydisp_inner]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = '(75.10/15 * t)* y / 10'
  [../]
  [./xdisp_outer]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 2
    function = '(68.8/15 * t)* x / 20'
  [../]
  [./ydisp_outer]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 2
    function = '(68.8/15 * t)* y / 20'
  [../]
  [./yfix]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0
  [../]
  [./xfix]
    type = DirichletBC
    variable = disp_x
    boundary = 3
    value = 0
  [../]
[]

[Preconditioning]
  active = 'smp'
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient

  solve_type = NEWTON

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'none'

  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  l_tol = 1e-4
  l_max_its = 50
  nl_max_its = 50

  dtmin = 1# 1e-5
  dt = 1
  end_time = 15
  #num_steps = 1
[]

[Outputs]
  exodus = true
  csv = true
[]
