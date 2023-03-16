[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
  nz = 0
  xmin = -12
  xmax = 12
  ymin = -12
  ymax = 12
  elem_type = QUAD4
[]

[GlobalParams]
  # let's output all material properties for demonstration purposes
  outputs = exodus

  # prefactor on the penalty function kernels. The higher this value is, the
  # more rigorously the constraint is enforced
  penalty = 1e3
[]

#
# These AuxVariables hold the directly calculated free energy density in the
# simulation cell. They are provided for visualization purposes.
#
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
  [../]

  #
  # Helper kernel to cpompute the gradient contribution from interfaces of order
  # parameters evolved using the ACMultiInterface kernel
  #
  [./cross_terms]
    type = CrossTermGradientFreeEnergy
    variable = cross_energy
    interfacial_vars = 'eta1 eta2 eta3'
    #
    # The interface coefficient matrix. This should be symmetrical!
    #
    kappa_names = 'kappa11 kappa12 kappa13
                   kappa21 kappa22 kappa23
                   kappa31 kappa32 kappa33'
  [../]
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
    #
    # We set up a smooth cradial concentrtaion gradient
    # The concentration will quickly change to adapt to the preset order
    # parameters eta1, eta2, and eta3
    #
    [./InitialCondition]
      type = SmoothCircleIC
      x1 = 0.0
      y1 = 0.0
      radius = 5.0
      invalue = 1.0
      outvalue = 0.01
      int_width = 10.0
    [../]
  [../]

  [./eta1]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      #
      # Note: this initial conditions sets up a _sharp_ interface. Ideally
      # we should start with a smooth interface with a width consistent
      # with the kappa parameter supplied for the given interface.
      #
      function = 'r:=sqrt(x^2+y^2);if(r<=4,1,0)'
    [../]
  [../]
  [./eta2]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = 'r:=sqrt(x^2+y^2);if(r>4&r<=7,1,0)'
    [../]
  [../]
  [./eta3]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = 'r:=sqrt(x^2+y^2);if(r>7,1,0)'
    [../]
  [../]
[]

[Kernels]
  #
  # Cahn-Hilliard kernel for the concentration variable.
  # Note that we are not using an interfcae kernel on this variable, but rather
  # rely on the interface width enforced on the order parameters. This allows us
  # to use a direct solve using the CahnHilliard kernel _despite_ only using first
  # order elements.
  #
  [./c_res]
    type = CahnHilliard
    variable = c
    f_name = F
    coupled_variables = 'eta1 eta2 eta3'
  [../]
  [./time]
    type = TimeDerivative
    variable = c
  [../]

  #
  # Order parameter eta1
  # Each order parameter is acted on by 4 kernels:
  #  1. The stock time derivative deta_i/dt kernel
  #  2. The Allen-Cahn kernel that takes a Dervative Material for the free energy
  #  3. A gradient interface kernel that includes cross terms
  #     see http://mooseframework.org/wiki/PhysicsModules/PhaseField/DevelopingModels/MultiPhaseModels/ACMultiInterface/
  #  4. A penalty contribution that forces the interface contributions h(eta)
  #     to sum up to unity
  #
  [./deta1dt]
    type = TimeDerivative
    variable = eta1
  [../]
  [./ACBulk1]
    type = AllenCahn
    variable = eta1
    coupled_variables = 'eta2 eta3 c'
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
  [./penalty1]
    type = SwitchingFunctionPenalty
    variable = eta1
    etas    = 'eta1 eta2 eta3'
    h_names = 'h1   h2   h3'
  [../]

  #
  # Order parameter eta2
  #
  [./deta2dt]
    type = TimeDerivative
    variable = eta2
  [../]
  [./ACBulk2]
    type = AllenCahn
    variable = eta2
    coupled_variables = 'eta1 eta3 c'
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
  [./penalty2]
    type = SwitchingFunctionPenalty
    variable = eta2
    etas    = 'eta1 eta2 eta3'
    h_names = 'h1   h2   h3'
  [../]

  #
  # Order parameter eta3
  #
  [./deta3dt]
    type = TimeDerivative
    variable = eta3
  [../]
  [./ACBulk3]
    type = AllenCahn
    variable = eta3
    coupled_variables = 'eta1 eta2 c'
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
  [./penalty3]
    type = SwitchingFunctionPenalty
    variable = eta3
    etas    = 'eta1 eta2 eta3'
    h_names = 'h1   h2   h3'
  [../]
[]

[BCs]
  [./Periodic]
    [./All]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Materials]
  # here we declare some of the model parameters: the mobilities and interface
  # gradient prefactors. For this example we use arbitrary numbers. In an actual simulation
  # physical mobilities would be used, and the interface gradient prefactors would
  # be readjusted to the free energy magnitudes.
  [./consts]
    type = GenericConstantMaterial
    prop_names  = 'M   kappa_c L1 L2 L3  kappa11 kappa12 kappa13 kappa21 kappa22 kappa23 kappa31 kappa32 kappa33'
    prop_values = '0.2 0.75    1  1  1   0.75    0.75    0.75    0.75    0.75    0.75    0.75    0.75    0.75   '
  [../]

  # This material sums up the individual phase contributions. It is written to the output file
  # (see GlobalParams section above) and can be used to check the constraint enforcement.
  [./etasummat]
    type = ParsedMaterial
    property_name = etasum
    material_property_names = 'h1 h2 h3'
    expression = 'h1+h2+h3'
  [../]

  # The phase contribution factors for each material point are computed using the
  # SwitchingFunctionMaterials. Each phase with an order parameter eta contributes h(eta)
  # to the global free energy density. h is a function that switches smoothly from 0 to 1
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

  # The barrier function adds a phase transformation energy barrier. It also
  # Drives order parameters toward the [0:1] interval to avoid negative or larger than 1
  # order parameters (these are set to 0 and 1 contribution by the switching functions
  # above)
  [./barrier]
    type = MultiBarrierFunctionMaterial
    etas = 'eta1 eta2 eta3'
  [../]

  # We use DerivativeParsedMaterials to specify three (very) simple free energy
  # expressions for the three phases. All necessary derivatives are built automatically.
  # In a real problem these expressions can be arbitrarily complex (or even provided
  # by custom kernels).
  [./phase_free_energy_1]
    type = DerivativeParsedMaterial
    property_name = F1
    expression = '(c-1)^2'
    coupled_variables = 'c'
  [../]
  [./phase_free_energy_2]
    type = DerivativeParsedMaterial
    property_name = F2
    expression = '(c-0.5)^2'
    coupled_variables = 'c'
  [../]
  [./phase_free_energy_3]
    type = DerivativeParsedMaterial
    property_name = F3
    expression = 'c^2'
    coupled_variables = 'c'
  [../]

  # The DerivativeMultiPhaseMaterial ties the phase free energies together into a global free energy.
  # http://mooseframework.org/wiki/PhysicsModules/PhaseField/DevelopingModels/MultiPhaseModels/
  [./free_energy]
    type = DerivativeMultiPhaseMaterial
    property_name = F
    # we use a constant free energy (GeneriConstantmaterial property Fx)
    fi_names = 'F1  F2  F3'
    hi_names = 'h1  h2  h3'
    etas     = 'eta1 eta2 eta3'
    coupled_variables = 'c'
    W = 1
  [../]
[]

[Postprocessors]
  # The total free energy of the simulation cell to observe the energy reduction.
  [./total_free_energy]
    type = ElementIntegralVariablePostprocessor
    variable = local_energy
  [../]

  # for testing we also monitor the total solute amount, which should be conserved.
  [./total_solute]
    type = ElementIntegralVariablePostprocessor
    variable = c
  [../]
[]

[Preconditioning]
  # This preconditioner makes sure the Jacobian Matrix is fully populated. Our
  # kernels compute all Jacobian matrix entries.
  # This allows us to use the Newton solver below.
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = 'bdf2'

  # Automatic differentiation provedes a _full_ Jacobian in this example
  # so we can safely use NEWTON for a fast solve
  solve_type = 'NEWTON'

  l_max_its = 15
  l_tol = 1.0e-6

  nl_max_its = 50
  nl_rel_tol = 1.0e-6
  nl_abs_tol = 1.0e-6

  start_time = 0.0
  end_time   = 150.0

  [./TimeStepper]
    type = SolutionTimeAdaptiveDT
    dt = 0.1
  [../]
[]

[Debug]
  # show_var_residual_norms = true
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
  [./table]
    type = CSV
    delimiter = ' '
  [../]
[]
