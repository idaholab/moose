#
# Martensitic transformation
# One structural order parameter (SOP) governed by AllenCahn Eqn.
# Chemical driving force described by Landau Polynomial
# Coupled with elasticity (Mechanics)
# Eigenstrain as a function of SOP
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
  [./eta]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 50
      y1 = 50
      radius = 10.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 5.0
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

  [./eta_bulk]
    type = AllenCahn
    variable = eta
    f_name = F
  [../]
  [./eta_interface]
    type = ACInterface
    variable = eta
    kappa_name = kappa_eta
  [../]
  [./time]
    type = TimeDerivative
    variable = eta
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
    args = 'eta'
    constant_names = 'A2 A3 A4'
    constant_expressions = '0.2 -12.6 12.4'
    function = A2/2*eta^2+A3/3*eta^3+A4/4*eta^4
    enable_jit = true
    derivative_order = 2
  [../]

  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '70 30 30 70 30 70 30 30 30'
    fill_method = symmetric9
  [../]

  [./stress]
    type = ComputeLinearElasticStress
  [../]

  [./var_dependence]
    type = DerivativeParsedMaterial
    function = eta
    args = 'eta'
    f_name = var_dep
    enable_jit = true
    derivative_order = 2
  [../]

  [./eigenstrain]
    type = ComputeVariableEigenstrain
    eigen_base = '0.1 0.1 0 0 0 0'
    prefactor = var_dep
    #outputs = exodus
    args = 'eta'
    eigenstrain_name = eigenstrain
  [../]

  [./strain]
    type = ComputeSmallStrain
    displacements = 'disp_x disp_y'
    eigenstrain_names = eigenstrain
  [../]

  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    args = 'eta'
    derivative_order = 2
  [../]

  [./free_energy]
    type = DerivativeSumMaterial
    f_name = F
    sum_materials = 'Fc Fe'
    args = 'eta'
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
