p_left = 1
T_left = 0.00349

p_right = 0.1
T_right = 0.00279

middle = 50

[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = ${fparse 2 * middle}
  ymin = 0
  ymax = 0.5
  zmin = 0
  zmax = 0.5
  nx = 1000
  ny = 1
  nz = 1
[]

[Modules]
  [FluidProperties]
    [fp]
      type = IdealGasFluidProperties
    []
  []
[]

[ICs]
  [pressure]
    type = FunctionIC
    variable = pressure
    function = 'if (x < ${middle}, ${p_left}, ${p_right})'
  []
  [temperature]
    type = FunctionIC
    variable = T_fluid
    function = 'if (x < ${middle}, ${T_left}, ${T_right})'
  []
[]

[Materials]
  [varmat]
    type = PrimitiveVarMaterial # Remove from NS Module; Make Cons. Vars Work
    pressure = pressure
    T_fluid = T_fluid
    vel_x = velocity_x
    vel_y = velocity_y
    vel_z = velocity_z
    fp = fp
  []
  [supgmat]
    type = NavierStokesSUPGMaterial
    fp = fp
    # use_PVT = true # Creates ***ERROR*** b/c unused
    porosity = porosity
  []
  [pronghorn_fluid_material]
    type = GeneralFluidProps
    pebble_diameter = 0.06 # dummy (Re not used in this test)
    fp = fp
    porosity = porosity
  []
  [properties]
    type = GenericConstantMaterial
    prop_names = 'alpha'
    prop_values = '0.0'
  []
  [tau]
    type = MatrixTau
    fp = fp
    advective_limit = compressible
  []
  [const_drags_mat]
    type = FunctionAnisotropicDragCoefficients
    Darcy_coefficient = '0 0 0'
    Forchheimer_coefficient = '0 0 0'
  []
  [kappa]
    type = KappaFluid
  []
[]

[Variables]
  [pressure]
  []
  # [velocity]
  #   initial_condition = 0
  # []
  [velocity_x]
    initial_condition = 0
  []
  [velocity_y]
    initial_condition = 0
  []
  [velocity_z]
    initial_condition = 0
  []
  [T_fluid]
  []
[]

[AuxVariables]
  [porosity]
    initial_condition = 1.0
  []
  [source]
    initial_condition = 0.0
  []
  [density]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [density_auxker]
    type = ADMaterialRealAux
    variable = density
    property = 'rho'
  []
[]

[Kernels]
  #inactive = 'mass_supg x_momentum_supg y_momentum_supg z_momentum_supg energy_supg'
  # conservation of mass
  [mass_time]
    type = CNSMassTimeDerivative
    variable = pressure
    porosity = porosity
  []
  [mass_space]
    type = CNSMassAdvectiveFlux
    # type = ConservativeAdvection
    variable = pressure
    porosity = porosity
  []
  [mass_supg]
    type = CNSSUPGKernel
    # type = NSSUPGMass
    variable = pressure
    var_num = 0
    porosity = porosity
  []

  # x-momentum
  [x_momentum_time]
    type = CNSMomentumTimeDerivative
    variable = velocity_x
    component = 0
    porosity = porosity
  []
  [x_momentum_space]
    type = CNSMomentumAdvectiveFlux
    # type = ConservativeAdvection
    variable = velocity_x
    component = 0
    porosity = porosity
  []
  [x_momentum_pressure]
    type = CNSMomentumPressureGradient
    # type = PressureGradient
    variable = velocity_x
    component = 0
    porosity = porosity
  [../]
  [./x_momentum_supg]
    type = CNSSUPGKernel
    # type = NSSUPGMomentum
    variable = velocity_x
    var_num = 1
    porosity = porosity
  [../]

  # y-momentum
  [y_momentum_time]
    type = CNSMomentumTimeDerivative
    variable = velocity_y
    component = 1
    porosity = porosity
  []
  [y_momentum_space]
    type = CNSMomentumAdvectiveFlux
    # type = ConservativeAdvection
    variable = velocity_y
    component = 1
    porosity = porosity
  []
  [y_momentum_pressure]
    type = CNSMomentumPressureGradient
    # type = PressureGradient
    variable = velocity_y
    component = 1
    porosity = porosity
  []
  [y_momentum_supg]
    type = CNSSUPGKernel
    # type = NSSUPGMomentum
    variable = velocity_y
    var_num = 2
    porosity = porosity
  []

  # z-momentum
  [z_momentum_time]
    type = CNSMomentumTimeDerivative
    variable = velocity_z
    component = 2
    porosity = porosity
  []
  [z_momentum_space]
    type = CNSMomentumAdvectiveFlux
    # type = ConservativeAdvection
    variable = velocity_z
    component = 2
    porosity = porosity
  []
  [z_momentum_pressure]
    type = CNSMomentumPressureGradient
    # type = PressureGradient
    variable = velocity_z
    component = 2
    porosity = porosity
  []
  [z_momentum_supg]
    type = CNSSUPGKernel
    # type = NSSUPGMomentum
    variable = velocity_z
    var_num = 3
    porosity = porosity
  []

  # fluid total energy
  [energy_time]
    type = CNSFluidEnergyTimeDerivative
    variable = T_fluid
    porosity = porosity
  []
  [energy_space]
    type = CNSFluidEnergyAdvectiveFlux
    variable = T_fluid
    porosity = porosity
  []
  [energy_diffusive]
    type = CNSFluidEnergyDiffusiveFlux
    # type = ADVectorDiffusion
    variable = T_fluid
    porosity = porosity
  []
  [energy_supg]
    type = CNSSUPGKernel
    # type = NSSUPGEnergy
    variable = T_fluid
    var_num = 4
    porosity = porosity
  []
[]

[BCs]
  [vel_y_walls]
    type = DirichletBC
    variable = velocity_y
    boundary = 'front back top bottom'
    value = 0.0
  []
  [vel_z_walls]
    type = DirichletBC
    variable = velocity_z
    boundary = 'front back top bottom'
    value = 0.0
  []

  [vel_x_walls]
    type = DirichletBC
    variable = velocity_x
    boundary = 'left right'
    value = 0.0
  []
[]

[Executioner]
  type = Transient

  solve_type = NEWTON

  dt = 1e-2
  end_time = 20

  l_max_its = 60
  nl_max_its = 40
  nl_rel_tol = 1e-6
  nl_abs_tol = 1.e-8
[]

[Outputs]
  exodus = true
[]
