# Things we learn from experimenting in 2D
# - mesh refinement needed in the fluid
# - time step size can be ~6s
# - very high Ra number. Realistic system would be easier,
#   but to get a solution now we need to up the viscosity
# - solver parameters: scaling notably, preconditioner

cp_multiplier = 1e-6
cp_water_multiplier = 1e-1
step_multiplier = 25

# this is in lieu of a turbulence model for simplicity
mu_multiplier = 1e3

mult = 2
nx = '${fparse 40 * mult}'
ny = '${fparse 32 * mult}'

[Mesh]
  [bulk]
    type = GeneratedMeshGenerator # Can generate simple lines, rectangles and rectangular prisms
    dim = 2
    nx = ${nx}
    ny = ${ny}
    xmax = 10 # m
    ymax = 8 # m
  []

  [create_inner_water]
    type = ParsedSubdomainMeshGenerator
    input = bulk
    combinatorial_geometry = '(x > 2 & x < 3 & y > 1 & y < 5)'
    block_id = 2
  []
  [create_right_water]
    type = ParsedSubdomainMeshGenerator
    input = create_inner_water
    combinatorial_geometry = '(x > 7 & x < 8 & y > 1 & y < 5)'
    block_id = 3
  []

  [hollow_concrete]
    type = ParsedSubdomainMeshGenerator
    input = create_right_water
    block_id = 1
    combinatorial_geometry = 'x > 3.2 & x < 6.8 & y > 1 & y < 5'
  []

  [rename_blocks]
    type = RenameBlockGenerator
    input = hollow_concrete
    old_block = '0 1 2 3'
    new_block = 'concrete cavity water water_inactive'
  []

  [add_water_concrete_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = rename_blocks
    primary_block = water
    paired_block = concrete
    new_boundary = 'water_boundary'
  []
  [add_inactive_water_concrete_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_water_concrete_interface
    paired_block = water_inactive
    primary_block = concrete
    new_boundary = 'inactive_water_boundary'
  []

  [add_inner_cavity]
    type = SideSetsBetweenSubdomainsGenerator
    input = add_inactive_water_concrete_interface
    primary_block = concrete
    paired_block = cavity
    new_boundary = 'inner_cavity'
  []

  [add_concrete_outer_boundary]
    type = RenameBoundaryGenerator
    input = add_inner_cavity
    old_boundary = 'left right bottom top'
    new_boundary = 'air_boundary air_boundary air_boundary ground'
  []

  [remove_cavity]
    type = BlockDeletionGenerator
    input = 'add_concrete_outer_boundary'
    block = cavity
  []

  [pin_node]
    type = BoundingBoxNodeSetGenerator
    input = remove_cavity
    new_boundary = 'pinned_node'
    bottom_left = '1.999 4.999 0'
    top_right = '2.0001 5.0001 0'
  []
  # [refine_water]
  #   type = RefineBlockGenerator
  #   input = pin_node
  #   refinement = '3'
  #   block = 'water'
  # []
  [refine_water_side]
    type = RefineSidesetGenerator
    input = pin_node
    refinement = '2'
    boundaries = 'water_boundary'
  []
  uniform_refine = 1
  # second_order = true
[]

[Variables]
  [T]
    # Adds a Linear Lagrange variable by default
    block = 'concrete'
    initial_condition = 300
    # [InitialCondition]
    #   type = FunctionIC
    #   function = '300 + 400*16 - 200*(x-4)^2*(y-6.5)^2*(z-3)^2'
    # []
  []

  [velocity]
    family = LAGRANGE_VEC
    block = 'water'
    # order = SECOND
  []
  [p]
    block = 'water'
  []
  [T_water]
    initial_condition = 300
    block = 'water'
    # order = SECOND
  []
[]

[ICs]
  [velocity]
    type = VectorConstantIC
    x_value = 1e-15
    y_value = 1e-15
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
    gravity = '0 -9.81 0'
    ref_temp = 'temp_ref'
  []
  [gravity]
    type = INSADGravityForce
    variable = velocity
    gravity = '0 -9.81 0'
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
    prop_values = '955.7 0.6 ${fparse cp_water_multiplier * 4181} ${fparse mu_multiplier * 7.98e-4} 30'
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

  [inactive_water_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = 'inactive_water_boundary'
    T_infinity = 300.0
    # The heat transfer coefficient should be obtained from a correlation
    heat_transfer_coefficient = 30
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
  off_diagonals_in_auto_scaling = true

  # petsc_options_iname = '-pc_type -pc_hypre_type'
  # petsc_options_value = 'hypre boomeramg'
  line_search = none
  petsc_options_iname = '-pc_type -pc_factor_shift_type'
  petsc_options_value = 'lu NONZERO'

  nl_abs_tol = 1e-8

  end_time = 1000
  start_time = -1

  # steady_state_tolerance = 1e-5
  # steady_state_detection = true

  [TimeStepper]
    type = FunctionDT
    function = 'if(t<0, 0.1,${fparse 0.25 * step_multiplier})'
  []
[]

[Outputs]
  exodus = true
[]

# [Debug]
#   show_var_residual_norms = true
# []

[Postprocessors]
  [T_max]
    type = ElementExtremeValue
    variable = T_water
    value_type = 'max'
    block = 'water'
  []
  [T_min]
    type = ElementExtremeValue
    variable = T_water
    value_type = 'min'
    block = 'water'
  []
  [Ra]
    type = RayleighNumber
    beta = 2.9e-3
    T_hot = T_max
    T_cold = T_min
    l = 4
    mu_ave = '${fparse mu_multiplier * 7.98e-4}'
    cp_ave = '${fparse cp_water_multiplier * 4181}'
    k_ave = 0.6
    gravity_magnitude = 9.81
    rho_ave = 955.7
  []
[]

[AuxVariables]
  [velocity_z]
  []
[]
