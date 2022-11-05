#
# Fig. 7 input for 10.1016/j.commatsci.2017.02.017
# D. Schwen et al./Computational Materials Science 132 (2017) 36-45
# Solid gray curve (1)
# Eigenstrain and elastic energies ar computed per phase and then interpolated.
# Supply the RADIUS parameter (10-35) on the command line to generate data
# for all curves in the plot.
#

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 32
  xmin = 0
  xmax = 100
  second_order = true
[]

[Problem]
  coord_type = RSPHERICAL
[]

[GlobalParams]
  displacements = 'disp_r'
[]

[Functions]
  [./diff]
    type = ParsedFunction
    expression = '${RADIUS}-pos_c'
    symbol_names = pos_c
    symbol_values = pos_c
  [../]
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
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[Variables]
  # Solute concentration variable
  [./c]
    [./InitialCondition]
      type = SmoothCircleIC
      invalue = 1
      outvalue = 0
      x1 = 0
      y1 = 0
      radius = ${RADIUS}
      int_width = 3
    [../]
  [../]
  [./w]
  [../]

  # Phase order parameter
  [./eta]
    [./InitialCondition]
      type = SmoothCircleIC
      invalue = 1
      outvalue = 0
      x1 = 0
      y1 = 0
      radius = ${RADIUS}
      int_width = 3
    [../]
  [../]

  # Mesh displacement
  [./disp_r]
    order = SECOND
  [../]

  [./Fe_fit]
    order = SECOND
  [../]
[]

[Kernels]
  # Set up stress divergence kernels
  [./TensorMechanics]
  [../]

  # Split Cahn-Hilliard kernels
  [./c_res]
    type = SplitCHParsed
    variable = c
    f_name = F
    args = 'eta'
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
  [./detadt]
    type = TimeDerivative
    variable = eta
  [../]
  [./ACBulk1]
    type = AllenCahn
    variable = eta
    args = 'c'
    mob_name = L
    f_name = F
  [../]
  [./ACInterface]
    type = ACInterface
    variable = eta
    mob_name = L
    kappa_name = kappa_eta
  [../]

  [./Fe]
    type = MaterialPropertyValue
    prop_name = Fe
    variable = Fe_fit
  [../]

  [./autoadjust]
    type = MaskedBodyForce
    variable = w
    function = diff
    mask = mask
  [../]
[]

[Materials]
  # declare a few constants, such as mobilities (L,M) and interface gradient prefactors (kappa*)
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'M   L   kappa_c kappa_eta'
    prop_values = '1.0 1.0 0.5     1'
  [../]

  # forcing function mask
  [./mask]
    type = ParsedMaterial
    property_name = mask
    expression = grad/dt
    material_property_names = 'grad dt'
  [../]
  [./grad]
    type = VariableGradientMaterial
    variable = c
    prop = grad
  [../]
  [./time]
    type = TimeStepMaterial
  [../]

  # global mechanical properties
  [./elasticity_tensor_1]
    type = ComputeElasticityTensor
    C_ijkl = '1 1'
    base_name = phase1
    fill_method = symmetric_isotropic
  [../]
  [./elasticity_tensor_2]
    type = ComputeElasticityTensor
    C_ijkl = '1 1'
    base_name = phase2
    fill_method = symmetric_isotropic
  [../]
  [./strain_1]
    type = ComputeRSphericalSmallStrain
    base_name = phase1
  [../]
  [./strain_2]
    type = ComputeRSphericalSmallStrain
    base_name = phase2
    eigenstrain_names = eigenstrain
  [../]
  [./stress_1]
    type = ComputeLinearElasticStress
    base_name = phase1
  [../]
  [./stress_2]
    type = ComputeLinearElasticStress
    base_name = phase2
  [../]

  # eigenstrain per phase
  [./eigenstrain2]
    type = ComputeEigenstrain
    eigen_base = '0.05 0.05 0.05 0 0 0'
    base_name = phase2
    eigenstrain_name = eigenstrain
  [../]

  # switching functions
  [./switching]
    type = SwitchingFunctionMaterial
    function_name = h
    eta = eta
    h_order = SIMPLE
  [../]
  [./barrier]
    type = BarrierFunctionMaterial
    eta = eta
  [../]

  # chemical free energies
  [./chemical_free_energy_1]
    type = DerivativeParsedMaterial
    property_name = Fc1
    expression = 'c^2'
    coupled_variables = 'c'
    derivative_order = 2
  [../]
  [./chemical_free_energy_2]
    type = DerivativeParsedMaterial
    property_name = Fc2
    expression = '(1-c)^2'
    coupled_variables = 'c'
    derivative_order = 2
  [../]

  # elastic free energies
  [./elastic_free_energy_1]
    type = ElasticEnergyMaterial
    f_name = Fe1
    args = ''
    base_name = phase1
    derivative_order = 2
  [../]
  [./elastic_free_energy_2]
    type = ElasticEnergyMaterial
    f_name = Fe2
    args = ''
    base_name = phase2
    derivative_order = 2
  [../]

  # per phase free energies
  [./free_energy_1]
    type = DerivativeSumMaterial
    property_name = F1
    sum_materials = 'Fc1 Fe1'
    coupled_variables = 'c'
    derivative_order = 2
  [../]
  [./free_energy_2]
    type = DerivativeSumMaterial
    property_name = F2
    sum_materials = 'Fc2 Fe2'
    coupled_variables = 'c'
    derivative_order = 2
  [../]

  # global chemical free energy
  [./global_free_energy]
    type = DerivativeTwoPhaseMaterial
    f_name = F
    fa_name = F1
    fb_name = F2
    eta = eta
    args = 'c'
    W = 4
  [../]

  # global stress
  [./global_stress]
    type = TwoPhaseStressMaterial
    base_A = phase1
    base_B = phase2
  [../]

  [./elastic_free_energy]
    type = DerivativeTwoPhaseMaterial
    f_name = Fe
    fa_name = Fe1
    fb_name = Fe2
    eta = eta
    args = 'c'
    W = 0
  [../]
[]

[BCs]
  [./left_r]
    type = DirichletBC
    variable = disp_r
    boundary = 'left'
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
    outputs = 'table console'
  [../]
  [./total_solute]
    type = ElementIntegralVariablePostprocessor
    variable = c
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'table console'
  [../]
  [./pos_c]
    type = FindValueOnLine
    start_point = '0 0 0'
    end_point = '100 0 0'
    v = c
    target = 0.582
    tol = 1e-8
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'table console'
  [../]
  [./pos_eta]
    type = FindValueOnLine
    start_point = '0 0 0'
    end_point = '100 0 0'
    v = eta
    target = 0.5
    tol = 1e-8
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'table console'
  [../]
  [./c_min]
    type = ElementExtremeValue
    value_type = min
    variable = c
    execute_on = 'INITIAL TIMESTEP_END'
    outputs = 'table console'
  [../]
[]

[VectorPostprocessors]
  [./line]
    type = LineValueSampler
    variable = 'Fe_fit c w'
    start_point = '0 0 0'
    end_point =   '100 0 0'
    num_points = 5000
    sort_by = x
    outputs = vpp
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
  nl_max_its = 15
  l_tol = 1.0e-4
  nl_rel_tol = 1.0e-8
  nl_abs_tol = 2.0e-9
  start_time = 0.0
  end_time = 100000.0

  [./TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 7
    iteration_window = 1
    dt = 1
  [../]

  [./Adaptivity]
    initial_adaptivity = 5
    interval = 10
    max_h_level = 5
    refine_fraction = 0.9
    coarsen_fraction = 0.1
  [../]
[]

[Outputs]
  print_linear_residuals = false
  perf_graph = true
  execute_on = 'INITIAL TIMESTEP_END'
  [./table]
    type = CSV
    delimiter = ' '
    file_base = radius_${RADIUS}/energy_pp
  [../]
  [./vpp]
    type = CSV
    delimiter = ' '
    sync_times = '10 50 100 500 1000 5000 10000 50000 100000'
    sync_only = true
    time_data = true
    file_base = radius_${RADIUS}/energy_vpp
  [../]
[]
