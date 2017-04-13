#
# Eigenstrain with Mortar gradient periodicity
#

[Mesh]
  type = MortarPeriodicMesh
  dim = 2
  nx = 50
  ny = 50
  xmin = -0.5
  xmax = 0.5
  ymin = -0.5
  ymax = 0.5
  periodic_directions = 'x y'

  [./MortarInterfaces]
    [./left_right]
      master = 1
      slave = 3
      subdomain = 10
    [../]
    [./up_down]
      master = 0
      slave = 2
      subdomain = 11
    [../]
  [../]
[]

[MeshModifiers]
  [./cnode]
    type = AddExtraNodeset
    coord = '0.0 0.0'
    new_boundary = 100
  [../]
  [./anode]
    type = AddExtraNodeset
    coord = '0.0 0.5'
    new_boundary = 101
  [../]
[]

[GlobalParams]
  derivative_order = 2
  enable_jit = true
  displacements = 'disp_x disp_y'
[]

# AuxVars to compute the free energy density for outputting
[AuxVariables]
  [./local_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./local_free_energy]
    type = TotalFreeEnergy
    block = 0
    execute_on = 'initial LINEAR'
    variable = local_energy
    interfacial_vars = 'c'
    kappa_names = 'kappa_c'
  [../]
[]

[Variables]
  # Solute concentration variable
  [./c]
    [./InitialCondition]
      type = RandomIC
      min = 0.49
      max = 0.51
    [../]
    block = 0
  [../]
  [./w]
    block = 0
  [../]

  # Mesh displacement
  [./disp_x]
    block = 0
  [../]
  [./disp_y]
    block = 0
  [../]

  # Lagrange multipliers for gradient component periodicity
  [./lm_left_right_xx]
    order = FIRST
    family = LAGRANGE
    block = 10
  [../]
  [./lm_left_right_xy]
    order = FIRST
    family = LAGRANGE
    block = 10
  [../]
  [./lm_left_right_yx]
    order = FIRST
    family = LAGRANGE
    block = 10
  [../]
  [./lm_left_right_yy]
    order = FIRST
    family = LAGRANGE
    block = 10
  [../]

  [./lm_up_down_xx]
    order = FIRST
    family = LAGRANGE
    block = 11
  [../]
  [./lm_up_down_xy]
    order = FIRST
    family = LAGRANGE
    block = 11
  [../]
  [./lm_up_down_yx]
    order = FIRST
    family = LAGRANGE
    block = 11
  [../]
  [./lm_up_down_yy]
    order = FIRST
    family = LAGRANGE
    block = 11
  [../]
[]

[Constraints]
  [./ud_disp_x_grad_x]
    type = EqualGradientConstraint
    variable = lm_up_down_xx
    interface = up_down
    component = 0
    master_variable = disp_x
  [../]
  [./ud_disp_x_grad_y]
    type = EqualGradientConstraint
    variable = lm_up_down_xy
    interface = up_down
    component = 1
    master_variable = disp_x
  [../]
  [./ud_disp_y_grad_x]
    type = EqualGradientConstraint
    variable = lm_up_down_yx
    interface = up_down
    component = 0
    master_variable = disp_y
  [../]
  [./ud_disp_y_grad_y]
    type = EqualGradientConstraint
    variable = lm_up_down_yy
    interface = up_down
    component = 1
    master_variable = disp_y
  [../]

  [./lr_disp_x_grad_x]
    type = EqualGradientConstraint
    variable = lm_left_right_xx
    interface = left_right
    component = 0
    master_variable = disp_x
  [../]
  [./lr_disp_x_grad_y]
    type = EqualGradientConstraint
    variable = lm_left_right_xy
    interface = left_right
    component = 1
    master_variable = disp_x
  [../]
  [./lr_disp_y_grad_x]
    type = EqualGradientConstraint
    variable = lm_left_right_yx
    interface = left_right
    component = 0
    master_variable = disp_y
  [../]
  [./lr_disp_y_grad_y]
    type = EqualGradientConstraint
    variable = lm_left_right_yy
    interface = left_right
    component = 1
    master_variable = disp_y
  [../]
[]

[Kernels]
  # Set up stress divergence kernels
  [./TensorMechanics]
  [../]

  # Cahn-Hilliard kernels
  [./c_dot]
    type = CoupledTimeDerivative
    variable = w
    v = c
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
[]

[Materials]
  # declare a few constants, such as mobilities (L,M) and interface gradient prefactors (kappa*)
  [./consts]
    type = GenericConstantMaterial
    block = '0 10 11'
    prop_names  = 'M   kappa_c'
    prop_values = '0.2 0.01   '
  [../]

  [./shear1]
    type = GenericConstantRankTwoTensor
    block = 0
    tensor_values = '0 0 0 0 0 0.5'
    tensor_name = shear1
  [../]
  [./shear2]
    type = GenericConstantRankTwoTensor
    block = 0
    tensor_values = '0 0 0 0 0 -0.5'
    tensor_name = shear2
  [../]
  [./expand3]
    type = GenericConstantRankTwoTensor
    block = 0
    tensor_values = '1 1 0 0 0 0'
    tensor_name = expand3
  [../]

  [./weight1]
    type = DerivativeParsedMaterial
    block = 0
    function = '0.3*c^2'
    f_name = weight1
    args = c
  [../]
  [./weight2]
    type = DerivativeParsedMaterial
    block = 0
    function = '0.3*(1-c)^2'
    f_name = weight2
    args = c
  [../]
  [./weight3]
    type = DerivativeParsedMaterial
    block = 0
    function = '4*(0.5-c)^2'
    f_name = weight3
    args = c
  [../]

  # matrix phase
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    block = 0
    C_ijkl = '1 1'
    fill_method = symmetric_isotropic
  [../]
  [./strain]
    type = ComputeSmallStrain
    block = 0
    displacements = 'disp_x disp_y'
    eigenstrain_names = eigenstrain
  [../]

  [./eigenstrain]
    type = CompositeEigenstrain
    block = 0
    tensors = 'shear1  shear2  expand3'
    weights = 'weight1 weight2 weight3'
    args = c
    eigenstrain_name = eigenstrain
  [../]

  [./stress]
    type = ComputeLinearElasticStress
    block = 0
  [../]

  # chemical free energies
  [./chemical_free_energy]
    type = DerivativeParsedMaterial
    block = 0
    f_name = Fc
    function = '4*c^2*(1-c)^2'
    args = 'c'
    outputs = exodus
    output_properties = Fc
  [../]

  # elastic free energies
  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    block = 0
    args = 'c'
    outputs = exodus
    output_properties = Fe
  [../]

  # free energy (chemical + elastic)
  [./free_energy]
    type = DerivativeSumMaterial
    block = 0
    f_name = F
    sum_materials = 'Fc Fe'
    args = 'c'
  [../]
[]

[BCs]
  [./Periodic]
    [./up_down]
      primary = top
      secondary = bottom
      translation = '0 -1 0'
      variable = 'c w'
    [../]
    [./left_right]
      primary = left
      secondary = right
      translation = '1 0 0'
      variable = 'c w'
    [../]
  [../]

  # fix center point location
  [./centerfix_x]
    type = PresetBC
    boundary = 100
    variable = disp_x
    value = 0
  [../]
  [./centerfix_y]
    type = PresetBC
    boundary = 100
    variable = disp_y
    value = 0
  [../]

  # fix side point x coordinate to inhibit rotation
  [./angularfix]
    type = PresetBC
    boundary = 101
    variable = disp_x
    value = 0
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

# We monitor the total free energy and the total solute concentration (should be constant)
[Postprocessors]
  [./total_free_energy]
    type = ElementIntegralVariablePostprocessor
    block = 0
    execute_on = 'initial TIMESTEP_END'
    variable = local_energy
  [../]
  [./total_solute]
    type = ElementIntegralVariablePostprocessor
    block = 0
    execute_on = 'initial TIMESTEP_END'
    variable = c
  [../]
  [./min]
    type = ElementExtremeValue
    block = 0
    execute_on = 'initial TIMESTEP_END'
    value_type = min
    variable = c
  [../]
  [./max]
    type = ElementExtremeValue
    block = 0
    execute_on = 'initial TIMESTEP_END'
    value_type = max
    variable = c
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = 'PJFNK'

  line_search = basic

  # mortar currently does not support MPI parallelization
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = ' lu       NONZERO               1e-10'

  l_max_its = 30
  nl_max_its = 12

  l_tol = 1.0e-4

  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0
  num_steps = 200

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.01
  [../]
[]

[Outputs]
  execute_on = 'timestep_end'
  print_linear_residuals = false
  exodus = true
  [./table]
    type = CSV
    delimiter = ' '
  [../]
[]
