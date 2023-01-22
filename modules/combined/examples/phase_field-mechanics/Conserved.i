#
# Example 1
# Illustrating the coupling between chemical and mechanical (elastic) driving forces.
# An oversized precipitate deforms under a uniaxial compressive stress
# Check the file below for comments and suggestions for parameter modifications.
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 50
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0
      y1 = 0
      radius = 25.0
      invalue = 1.0
      outvalue = 0.0
      int_width = 50.0
    [../]
  [../]
  [./w]
    order = FIRST
    family = LAGRANGE
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

  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    kappa_name = kappa_c
    w = w
  [../]
  [./w_res]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]
  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
[]

#
# The AuxVariables and AuxKernels below are added to visualize the xx and yy stress tensor components
#
[AuxVariables]
  [./sigma11_aux]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./sigma22_aux]
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
    index_i = 1
    index_j = 1
    variable = sigma22_aux
  [../]
[]

[Materials]
  [./pfmobility]
    type = GenericConstantMaterial
    prop_names  = 'M kappa_c'
    prop_values = '1 5'
    block = 0
    #kappa = 0.1
    #mob = 1e-3
  [../]

  # simple chemical free energy with a miscibility gap
  [./chemical_free_energy]
    type = DerivativeParsedMaterial
    block = 0
    property_name = Fc
    coupled_variables = 'c'
    constant_names       = 'barr_height  cv_eq'
    constant_expressions = '0.1          1.0e-2'
    expression = 16*barr_height*(c-cv_eq)^2*(1-cv_eq-c)^2
    enable_jit = true
    derivative_order = 2
  [../]

  # undersized solute (voidlike)
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    # lambda, mu values
    C_ijkl = '7 7'
    # Stiffness tensor is created from lambda=7, mu=7 using symmetric_isotropic fill method
    fill_method = symmetric_isotropic
    # See RankFourTensor.h for details on fill methods
    # '15 15' results in a high stiffness (the elastic free energy will dominate)
    # '7 7' results in a low stiffness (the chemical free energy will dominate)
  [../]
  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]
  [./var_dependence]
    type = DerivativeParsedMaterial
    block = 0
    # eigenstrain coefficient
    # -0.1 will result in an undersized precipitate
    #  0.1 will result in an oversized precipitate
    expression = 0.1*c
    coupled_variables = c
    f_name = var_dep
    enable_jit = true
    derivative_order = 2
  [../]
  [./eigenstrain]
    type = ComputeVariableEigenstrain
    block = 0
    eigen_base = '1 1 1 0 0 0'
    prefactor = var_dep
    #outputs = exodus
    args = 'c'
    eigenstrain_name = eigenstrain
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y'
    eigenstrain_names = eigenstrain
  [../]
  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    block = 0
    args = 'c'
    derivative_order = 2
  [../]

  # Sum up chemical and elastic contributions
  [./free_energy]
    type = DerivativeSumMaterial
    block = 0
    property_name = F
    sum_materials = 'Fc Fe'
    coupled_variables = 'c'
    derivative_order = 2
  [../]
[]

[BCs]
  [./bottom_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0
  [../]
  [./top_y]
    type = DirichletBC
    variable = disp_y
    boundary = 'top'

    # prescribed displacement
    # -5 will result in a compressive stress
    #  5 will result in a tensile stress
    value = -5
  [../]
  [./left_x]
    type = DirichletBC
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

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type  -sub_pc_type '
  petsc_options_value = 'asm       lu'

  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10
  start_time = 0.0
  num_steps = 200

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 1
  [../]
[]

[Outputs]
  exodus = true
[]
