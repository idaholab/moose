#
# Martensitic transformation
# Chemical driving force described by Landau Polynomial
# Coupled with elasticity (Mechanics)
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  nz = 0
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 100
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./eta1]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = RndBoundingBoxIC
      x1 = 0
      y1 = 0
      x2 = 100
      y2 = 100
      mx_invalue = 0.1
      mn_invalue = 0
      mx_outvalue = 0
      mn_outvalue = 0
    [../]
  [../]
  [./eta2]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = RndBoundingBoxIC
      x1 = 0
      y1 = 0
      x2 = 100
      y2 = 100
      mx_invalue = 0.1
      mn_invalue = 0
      mx_outvalue = 0
      mn_outvalue = 0
    [../]
  [../]
  [./disp_x]
    order = FIRST
    family = LAGRANGE
  [../]
  [./disp_y]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./TensorMechanics]
    displacements = 'disp_x disp_y'
  [../]

  [./eta_bulk1]
    type = AllenCahn
    variable = eta1
    args = 'eta2'
    f_name = F
  [../]
  [./eta_bulk2]
    type = AllenCahn
    variable = eta2
    args = 'eta1'
    f_name = F
  [../]
  [./eta_interface1]
    type = ACInterface
    variable = eta1
    kappa_name = kappa_eta
  [../]
  [./eta_interface2]
    type = ACInterface
    variable = eta2
    kappa_name = kappa_eta
  [../]

  [./deta1dt]
    type = TimeDerivative
    variable = eta1
  [../]
  [./deta2dt]
    type = TimeDerivative
    variable = eta2
  [../]
[]

#
# Try visualizing the stress tensor components as done in Conserved.i
#

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'L kappa_eta'
    prop_values = '1 1'
  [../]

  [./chemical_free_energy]
    type = DerivativeParsedMaterial
    f_name = Fc
    args = 'eta1 eta2'
    constant_names = 'A2 A3 A4'
    constant_expressions = '0.2 -12.6 12.4'
    function = A2/2*(eta1^2+eta2^2)+A3/3*(eta1^3+eta2^3)+A4/4*(eta1^2+eta2^2)^2
    enable_jit = true
    derivative_order = 2
  [../]

  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '700 300 300 700 300 700 300 300 300'
    fill_method = symmetric9
  [../]

  [./stress]
    type = ComputeLinearElasticStress
  [../]

  [./var_dependence1]
    type = DerivativeParsedMaterial
    f_name = var_dep1
    args = 'eta1'
    function = eta1
    enable_jit = true
    derivative_order = 2
  [../]
  [./var_dependence2]
    type = DerivativeParsedMaterial
    f_name = var_dep2
    args = 'eta2'
    function = eta2
    enable_jit = true
    derivative_order = 2
  [../]

  [./eigenstrain1]
    type = ComputeVariableEigenstrain
    eigen_base = '0.1 -0.1 0 0 0 0'
    prefactor = var_dep1
    args = 'eta1'
    eigenstrain_name = eigenstrain1
  [../]

  [./eigenstrain2]
    type = ComputeVariableEigenstrain
    eigen_base = '-0.1 0.1 0 0 0 0'
    prefactor = var_dep2
    args = 'eta2'
    eigenstrain_name = eigenstrain2
  [../]

  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y'
    eigenstrain_names = 'eigenstrain1 eigenstrain2'
  [../]

  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    args = 'eta1 eta2'
    derivative_order = 2
  [../]

  [./totol_free_energy]
    type = DerivativeSumMaterial
    f_name = F
    sum_materials = 'Fc Fe'
    args = 'eta1 eta2'
    derivative_order = 2
  [../]
[]

[AuxVariables]
  [./sigma11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./sigma12_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]
[AuxKernels]
  [./matl_sigma11]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 0
    variable = sigma11_aux
  [../]
  [./matl_sigma22]
    type = RankTwoAux
    rank_two_tensor = stress
    index_i = 0
    index_j = 1
    variable = sigma12_aux
    execute_on = 'timestep_end initial'
  [../]
[]

[BCs]
  [./bottom_y]
    type = PresetBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./top_y]
    type = PresetBC
    variable = disp_y
    boundary = 'top'
    value = 0
  [../]
  [./left_y]
    type = PresetBC
    variable = disp_y
    boundary = 'left'
    value = 0
  [../]
  [./right_y]
      type = PresetBC
      variable = disp_y
      boundary = 'right'
      value = 0
  [../]
  [./top_x]
    type = PresetBC
    variable = disp_x
    boundary = 'top'
    value = 0
  [../]
  [./bottom_x]
      type = PresetBC
      variable = disp_x
      boundary = 'bottom'
      value = 0
    [../]
      [./right_x]
    type = PresetBC
    variable = disp_x
    boundary = 'right'
    value = 0
  [../]
  [./left_x]
    type = PresetBC
    variable = disp_x
    boundary = 'left'
    value = 0
  [../]
[]

[Preconditioning]
  # active = ' '
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  # this gives best performance on 4 cores
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type  -sub_pc_type '
  petsc_options_value = 'asm       lu'

  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10
  start_time = 0.0
  num_steps = 10

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.3
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
