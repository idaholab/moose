#
# Fig. 8 input for 10.1016/j.commatsci.2017.02.017
# D. Schwen et al./Computational Materials Science 132 (2017) 36-45
# Two growing particles with differnet anisotropic Eigenstrains
#

[Mesh]
  [./gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 80
    ny = 40
    xmin = -20
    xmax = 20
    ymin = 0
    ymax = 20
    elem_type = QUAD4
  [../]
  [./cnode]
    type = ExtraNodesetGenerator
    input = gen
    coord = '0.0 0.0'
    new_boundary = 100
    tolerance = 0.1
  [../]
[]

[GlobalParams]
  # CahnHilliard needs the third derivatives
  derivative_order = 3
  enable_jit = true
  displacements = 'disp_x disp_y'
  int_width = 1
[]

# AuxVars to compute the free energy density for outputting
[AuxVariables]
  [./local_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./cross_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [./local_free_energy]
    type = TotalFreeEnergy
    variable = local_energy
    interfacial_vars = 'c'
    kappa_names = 'kappa_c'
    additional_free_energy = cross_energy
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./cross_terms]
    type = CrossTermGradientFreeEnergy
    variable = cross_energy
    interfacial_vars = 'eta1 eta2 eta3'
    kappa_names = 'kappa11 kappa12 kappa13
                   kappa21 kappa22 kappa23
                   kappa31 kappa32 kappa33'
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

# particle x positions and radius
P1X=8
P2X=-4
PR=2

[Variables]
  # Solute concentration variable
  [./c]
    [./InitialCondition]
      type = SpecifiedSmoothCircleIC
      x_positions = '${P1X} ${P2X}'
      y_positions = '0 0'
      z_positions = '0 0'
      radii = '${PR} ${PR}'
      outvalue = 0.5
      invalue = 0.9
    [../]
  [../]
  [./w]
  [../]

  # Order parameter for the Matrix
  [./eta1]
    [./InitialCondition]
      type = SpecifiedSmoothCircleIC
      x_positions = '${P1X} ${P2X}'
      y_positions = '0 0'
      z_positions = '0 0'
      radii = '${PR} ${PR}'
      outvalue = 1.0
      invalue = 0.0
    [../]
  [../]
  # Order parameters for the 2 different inclusion orientations
  [./eta2]
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = ${P2X}
      y1 = 0
      radius = ${PR}
      invalue = 1.0
      outvalue = 0.0
    [../]
  [../]
  [./eta3]
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = ${P1X}
      y1 = 0
      radius = ${PR}
      invalue = 1.0
      outvalue = 0.0
    [../]
  [../]

  # Lagrange-multiplier
  [./lambda]
    initial_condition = 1.0
  [../]
[]

[Modules]
  [./TensorMechanics]
    [./Master]
      [./all]
        add_variables = true
        strain = SMALL
        eigenstrain_names = eigenstrain
      [../]
    [../]
  [../]
[]

[Kernels]
  # Split Cahn-Hilliard kernels
  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    args = 'eta1 eta2 eta3'
    kappa_name = kappa_c
    w = w
  [../]
  [./wres]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]
  [./time]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]

  # Allen-Cahn and Lagrange-multiplier constraint kernels for order parameter 1
  [./deta1dt]
    type = TimeDerivative
    variable = eta1
  [../]
  [./ACBulk1]
    type = AllenCahn
    variable = eta1
    args = 'eta2 eta3 c'
    mob_name = L1
    f_name = F
  [../]
  [./ACInterface1]
    type = ACMultiInterface
    variable = eta1
    etas = 'eta1 eta2 eta3'
    mob_name = L1
    kappa_names = 'kappa11 kappa12 kappa13'
  [../]
  [./lagrange1]
    type = SwitchingFunctionConstraintEta
    variable = eta1
    h_name   = h1
    lambda = lambda
  [../]

  # Allen-Cahn and Lagrange-multiplier constraint kernels for order parameter 2
  [./deta2dt]
    type = TimeDerivative
    variable = eta2
  [../]
  [./ACBulk2]
    type = AllenCahn
    variable = eta2
    args = 'eta1 eta3 c'
    mob_name = L2
    f_name = F
  [../]
  [./ACInterface2]
    type = ACMultiInterface
    variable = eta2
    etas = 'eta1 eta2 eta3'
    mob_name = L2
    kappa_names = 'kappa21 kappa22 kappa23'
  [../]
  [./lagrange2]
    type = SwitchingFunctionConstraintEta
    variable = eta2
    h_name   = h2
    lambda = lambda
  [../]

  # Allen-Cahn and Lagrange-multiplier constraint kernels for order parameter 3
  [./deta3dt]
    type = TimeDerivative
    variable = eta3
  [../]
  [./ACBulk3]
    type = AllenCahn
    variable = eta3
    args = 'eta1 eta2 c'
    mob_name = L3
    f_name = F
  [../]
  [./ACInterface3]
    type = ACMultiInterface
    variable = eta3
    etas = 'eta1 eta2 eta3'
    mob_name = L3
    kappa_names = 'kappa31 kappa32 kappa33'
  [../]
  [./lagrange3]
    type = SwitchingFunctionConstraintEta
    variable = eta3
    h_name   = h3
    lambda = lambda
  [../]

  # Lagrange-multiplier constraint kernel for lambda
  [./lagrange]
    type = SwitchingFunctionConstraintLagrange
    variable = lambda
    etas    = 'eta1 eta2 eta3'
    h_names = 'h1   h2   h3'
    epsilon = 1e-6
  [../]
[]

[Materials]
  # declare a few constants, such as mobilities (L,M) and interface gradient prefactors (kappa*)
  [./consts]
    type = GenericConstantMaterial
    block = 0
    prop_names  = 'M   kappa_c  L1 L2 L3  kappa11 kappa12 kappa13 kappa21 kappa22 kappa23 kappa31 kappa32 kappa33'
    prop_values = '0.2 0.5      1  1  1   2.00    2.00    2.00    2.00    2.00    2.00    2.00    2.00    2.00   '
  [../]

  # We use this to output the level of constraint enforcement
  # ideally it should be 0 everywhere, if the constraint is fully enforced
  [./etasummat]
    type = ParsedMaterial
    property_name = etasum
    coupled_variables = 'eta1 eta2 eta3'
    material_property_names = 'h1 h2 h3'
    expression = 'h1+h2+h3-1'
    outputs = exodus
  [../]

  # This parsed material creates a single property for visualization purposes.
  # It will be 0 for phase 1, -1 for phase 2, and 1 for phase 3
  [./phasemap]
    type = ParsedMaterial
    property_name = phase
    coupled_variables = 'eta2 eta3'
    expression = 'if(eta3>0.5,1,0)-if(eta2>0.5,1,0)'
    outputs = exodus
  [../]

  # global mechanical properties
  [./elasticity_tensor]
    type = ComputeElasticityTensor
    C_ijkl = '400 400'
    fill_method = symmetric_isotropic
  [../]
  [./stress]
    type = ComputeLinearElasticStress
  [../]

  # eigenstrain
  [./eigenstrain_2]
    type = GenericConstantRankTwoTensor
    tensor_name = s2
    tensor_values = '0 -0.05 0  0 0 0'
  [../]
  [./eigenstrain_3]
    type = GenericConstantRankTwoTensor
    tensor_name = s3
    tensor_values =  '-0.05 0 0  0 0 0'
  [../]
  [./eigenstrain]
    type = CompositeEigenstrain
    weights = 'h2 h3'
    tensors = 's2 s3'
    args = 'eta2 eta3'
    eigenstrain_name = eigenstrain
  [../]

  # switching functions
  [./switching1]
    type = SwitchingFunctionMaterial
    function_name = h1
    eta = eta1
    h_order = SIMPLE
  [../]
  [./switching2]
    type = SwitchingFunctionMaterial
    function_name = h2
    eta = eta2
    h_order = SIMPLE
  [../]
  [./switching3]
    type = SwitchingFunctionMaterial
    function_name = h3
    eta = eta3
    h_order = SIMPLE
  [../]

  [./barrier]
    type = MultiBarrierFunctionMaterial
    etas = 'eta1 eta2 eta3'
  [../]

  # chemical free energies
  [./chemical_free_energy_1]
    type = DerivativeParsedMaterial
    property_name = Fc1
    expression = '4*c^2'
    coupled_variables = 'c'
    derivative_order = 2
  [../]
  [./chemical_free_energy_2]
    type = DerivativeParsedMaterial
    property_name = Fc2
    expression = '(c-0.9)^2-0.4'
    coupled_variables = 'c'
    derivative_order = 2
  [../]
  [./chemical_free_energy_3]
    type = DerivativeParsedMaterial
    property_name = Fc3
    expression = '(c-0.9)^2-0.5'
    coupled_variables = 'c'
    derivative_order = 2
  [../]

  # global chemical free energy
  [./chemical_free_energy]
    type = DerivativeMultiPhaseMaterial
    f_name = Fc
    fi_names = 'Fc1  Fc2  Fc3'
    hi_names = 'h1  h2  h3'
    etas     = 'eta1 eta2 eta3'
    coupled_variables = 'c'
    W = 3
  [../]

  # global elastic free energy
  [./elastic_free_energy]
    type = ElasticEnergyMaterial
    f_name = Fe
    args = 'eta2 eta3'
    outputs = exodus
    output_properties = Fe
    derivative_order = 2
  [../]

  # Penalize phase 2 and 3 coexistence
  [./multi_phase_penalty]
    type = DerivativeParsedMaterial
    property_name = Fp
    expression = '50*(eta2*eta3)^2'
    coupled_variables = 'eta2 eta3'
    derivative_order = 2
    outputs = exodus
    output_properties = Fp
  [../]

  # free energy
  [./free_energy]
    type = DerivativeSumMaterial
    property_name = F
    sum_materials = 'Fc Fe Fp'
    coupled_variables = 'c eta1 eta2 eta3'
    derivative_order = 2
  [../]
[]

[BCs]
  # fix center point location
  [./centerfix_x]
    type = DirichletBC
    boundary = 100
    variable = disp_x
    value = 0
  [../]

  # fix side point x coordinate to inhibit rotation
  [./angularfix]
    type = DirichletBC
    boundary = bottom
    variable = disp_y
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
    variable = local_energy
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./total_solute]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2

  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -sub_pc_type'
  petsc_options_value = 'asm      lu'

  l_max_its = 30
  nl_max_its = 10
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10
  start_time = 0.0
  end_time = 12.0

  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 8
    iteration_window = 1
    dt = 0.01
  [../]
[]

[Outputs]
  print_linear_residuals = false
  execute_on = 'INITIAL TIMESTEP_END'
  exodus = true
  [./table]
    type = CSV
    delimiter = ' '
  [../]
[]

[Debug]
  # show_var_residual_norms = true
[]
