cp_multiplier = 1e-6
cp_water_multiplier = 1e-6

[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = '../../step03_boundary_conditions/inputs/mesh_in.e'
  []

  [pin_node]
    type = ExtraNodesetGenerator
    new_boundary = 'pinned_node'
    nodes = '2000'
    input = fmg
  []

  [add_inner_water]
    type = SideSetsFromBoundingBoxGenerator
    input = pin_node
    boundaries_old = 'water_boundary'
    boundary_new = water_boundary_inner
    bottom_left = '2.5 2.5 1'
    top_right = '6.6 10.5 5'
    location = INSIDE
  []
  [add_outer_water]
    type = SideSetsFromBoundingBoxGenerator
    input = add_inner_water
    boundaries_old = 'water_boundary'
    boundary_new = water_boundary_outer
    bottom_left = '2.5 2.5 1'
    top_right = '6.6 10.5 5'
    location = OUTSIDE
  []

  [refine_water]
    type = RefineBlockGenerator
    input = add_outer_water
    refinement = '0'
    block = 'water'
  []
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete'
    # [InitialCondition]
    #   type = FunctionIC
    #   function = '300 + 400*16 - 200*(x-4)^2*(y-6.5)^2*(z-3)^2'
    # []
  []

  [velocity]
    family = LAGRANGE_VEC
    block = 'water'
  []
  [p]
    block = 'water'
  []
  [T_water]
    initial_condition = 300
    block = 'water'
  []
[]

[ICs]
  [velocity]
    type = VectorConstantIC
    x_value = 1e-15
    y_value = 1e-15
    z_value = 1e-15
    variable = velocity
  []
[]

[Kernels]
  # Solid heat conduction
  [diffusion_concrete]
    type = ADHeatConduction
    variable = T
  []
  [time_derivative]
    type = ADHeatConductionTimeDerivative
    variable = T
  []

  # Fluid flow equations: mass
  [mass]
    type = INSADMass
    variable = p
    block = 'water'
  []
  [mass_pspg]
    type = INSADMassPSPG
    variable = p
  []

  # # Fluid flow: momentum
  [momentum_time]
    type = INSADMomentumTimeDerivative
    variable = velocity
  []
  [momentum_convection]
    type = INSADMomentumAdvection
    variable = velocity
  []
  [momentum_viscous]
    type = INSADMomentumViscous
    variable = velocity
  []
  [momentum_pressure]
    type = INSADMomentumPressure
    variable = velocity
    pressure = p
    integrate_p_by_parts = true
  []
  [buoyancy]
    type = INSADBoussinesqBodyForce
    variable = velocity
    temperature = T_water
    gravity = '0 0 -9.81 '
    ref_temp = 'temp_ref'
  []
  [gravity]
    type = INSADGravityForce
    variable = velocity
    gravity = '0 0 -9.81'
  []
  [momentum_supg]
    type = INSADMomentumSUPG
    variable = velocity
    velocity = velocity
  []

  # # Fluid flow: energy
  [temperature_time]
    type = INSADHeatConductionTimeDerivative
    variable = T_water
  []
  [temperature_advection]
    type = INSADEnergyAdvection
    variable = T_water
  []
  [temperature_conduction]
    type = ADHeatConduction
    variable = T_water
    thermal_conductivity = 'k'
  []
  [temperature_supg]
    type = INSADEnergySUPG
    variable = T_water
    velocity = velocity
  []
[]

[Materials]
  [concrete]
    type = ADHeatConductionMaterial
    block = 'concrete'
    temp = 'T'
    # we specify a function of time, temperature is passed as the time argument
    # in the material
    thermal_conductivity_temperature_function = '2.25 + 0.001 * t'
    specific_heat = '${fparse cp_multiplier * 1170}'
  []
  [density]
    type = ADGenericConstantMaterial
    block = 'concrete'
    prop_names = 'density'
    prop_values = '2400' # kg / m3
  []

  [water]
    type = ADGenericConstantMaterial
    block = 'water'
    prop_names = 'rho    k     cp      mu alpha_wall'
    prop_values = '955.7 0.6 ${fparse cp_water_multiplier * 4181} 7.98e-4 30'
  []
  [boussinesq_params]
    type = ADGenericConstantMaterial
    prop_names = 'alpha '
    prop_values = '2.9e-3'
  []
  [boussinesq_params_ad]
    type = GenericConstantMaterial
    prop_names = 'temp_ref'
    prop_values = ' 300'
  []
  [ins_mat]
    type = INSADStabilized3Eqn
    block = 'water'
    velocity = velocity
    pressure = p
    temperature = T_water
  []
[]

[BCs]
  # Solid
  [from_reactor]
    type = FunctionNeumannBC
    variable = T
    boundary = inner_cavity
    # 100 kW reactor, 108 m2 cavity area
    # ramp up over 10s
    function = '1e5 / 108 * min(t / 10, 1)'
  []
  [air_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = 'air_boundary'
    T_infinity = 300.0
    # The heat transfer coefficient should be obtained from a correlation
    heat_transfer_coefficient = 10
  []
  [ground]
    type = DirichletBC
    variable = T
    value = 300
    boundary = 'ground'
  []

  # Water
  [no_slip]
    type = VectorFunctionDirichletBC
    variable = velocity
    boundary = 'water_boundary'
  []
  [pressure_pin]
    type = DirichletBC
    variable = p
    boundary = 'pinned_node'
    value = 0
  []
[]

[InterfaceKernels]
  [water_concrete_interface]
    type = ConjugateHeatTransfer
    variable = T_water
    T_fluid = T_water
    neighbor_var = 'T'
    boundary = 'water_boundary'
    htc = 'alpha_wall'
  []
[]

[Problem]
  kernel_coverage_check = false
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  automatic_scaling = true

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
  line_search = none
  # petsc_options_iname = '-pc_type -pc_factor_shift_type'
  # petsc_options_value = 'lu NONZERO'

  l_abs_tol = 1e-9
  nl_abs_tol = 1e-8

  end_time = 100
  dt = 0.25
  start_time = -1

  # steady_state_tolerance = 1e-5
  # steady_state_detection = true

  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0,0.1,0.25)'
  []
[]

[Outputs]
  exodus = true
[]
