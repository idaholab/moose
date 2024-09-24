#
# Martensitic transformation
# One structural order parameter (SOP) governed by AllenCahn Eqn.
# Chemical driving force described by Landau Polynomial
# Coupled with elasticity (Mechanics)
# Eigenstrain as a function of SOP
#

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 100
  ny = 100
  xmin = 0
  xmax = 100
  ymin = 0
  ymax = 100

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
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    add_variables = true
    generate_output = 'stress_xx stress_yy'
    eigenstrain_names = 'eigenstrain'
  [../]
[]

[Kernels]
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

[Materials]
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'L kappa_eta'
    prop_values = '1 1'
  [../]

  [./chemical_free_energy]
    type = DerivativeParsedMaterial
    property_name = Fc
    coupled_variables = 'eta'
    constant_names = 'A2 A3 A4'
    constant_expressions = '0.2 -12.6 12.4'
    expression = A2/2*eta^2+A3/3*eta^3+A4/4*eta^4
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
    expression = eta
    coupled_variables = 'eta'
    property_name = var_dep
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

  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    args = 'eta'
    derivative_order = 2
  [../]

  [./free_energy]
    type = DerivativeSumMaterial
    property_name = F
    sum_materials = 'Fc Fe'
    coupled_variables = 'eta'
    derivative_order = 2
  [../]
[]

[BCs]
  [./all_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'top bottom left right'
    value = 0
  [../]
  [./all_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'top bottom left right'
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
    type = IterationAdaptiveDT
    optimal_iterations = 9
    iteration_window = 2
    growth_factor = 1.1
    cutback_factor = 0.75
    dt = 0.3
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]
