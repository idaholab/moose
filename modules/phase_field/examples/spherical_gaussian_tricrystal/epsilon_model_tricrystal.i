#  Epsilon Model - Tricrystal

[Mesh]
  [Base_mesh]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 80
    ny = 40
    xmin = 0
    xmax = 800
    ymin = 0
    ymax = 400
    elem_type = QUAD4
  []
[]

[GlobalParams]
  op_num = 3
  var_name_base = gr
  order = CONSTANT
  family = MONOMIAL
[]

[Variables]
  [PolycrystalVariables]
    order = FIRST
    family = LAGRANGE
  []
[]

[UserObjects]
  [grain_tracker]
    type = GrainTracker
    variable = 'gr0 gr1 gr2'
    threshold = 0.2
    connecting_threshold = 0.08
    compute_var_to_feature_map = true
  []
[]

[ICs]
  [InitialCondition_gr0]
    type = TricrystalTripleJunctionIC
    op_index = 1
    variable = gr0
    theta1 = 140
    junction = '165 200 0'
  []
  [InitialCondition_gr1]
    type = TricrystalTripleJunctionIC
    op_index = 2
    variable = gr1
    theta2 = 106.5
    junction = '165 200 0'
  []
  [InitialCondition_gr2]
    type = TricrystalTripleJunctionIC
    op_index = 3
    variable = gr2
    theta1 = 140
    theta2 = 106.5
    junction = '165 200 0'
  []
[]

[AuxVariables]
  [bnds]
    order = FIRST
    family = LAGRANGE
  []
  [unique_grains]
  []
  [var_indices]
  []
  [ghost_regions]
  []
  [proc]
  []
  [bounds_dummy0]
    order = FIRST
    family = LAGRANGE
  []
  [bounds_dummy1]
    order = FIRST
    family = LAGRANGE
  []
  [bounds_dummy2]
    order = FIRST
    family = LAGRANGE
  []
  [free_energy]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Bounds]
  [d_upper_bound0]
    type = ConstantBounds
    variable = bounds_dummy0
    bounded_variable = gr0
    bound_type = upper
    bound_value = 1.0
  []
  [d_lower_bound0]
    type = ConstantBounds
    variable = bounds_dummy0
    bounded_variable = gr0
    bound_type = lower
    bound_value = 0.0
  []
  [d_upper_bound1]
    type = ConstantBounds
    variable = bounds_dummy1
    bounded_variable = gr1
    bound_type = upper
    bound_value = 1.0
  []
  [d_lower_bound1]
    type = ConstantBounds
    variable = bounds_dummy1
    bounded_variable = gr1
    bound_type = lower
    bound_value = 0.0
  []
  [d_upper_bound2]
    type = ConstantBounds
    variable = bounds_dummy2
    bounded_variable = gr2
    bound_type = upper
    bound_value = 1.0
  []
  [d_lower_bound2]
    type = ConstantBounds
    variable = bounds_dummy2
    bounded_variable = gr2
    bound_type = lower
    bound_value = 0.0
  []
[]

[Kernels]
  # Set up kernels for Epsilon model
  [SphericalGaussianKernel]
    model_type = EPSILON
  []
[]

[AuxKernels]
  [bnds_aux]
    type = BndsCalcAux
    variable = bnds
    execute_on = timestep_end
  []
  [unique_grains]
    type = FeatureFloodCountAux
    variable = unique_grains
    flood_counter = grain_tracker
    field_display = UNIQUE_REGION
    execute_on = 'initial timestep_end'
  []
  [var_indices]
    type = FeatureFloodCountAux
    variable = var_indices
    flood_counter = grain_tracker
    field_display = VARIABLE_COLORING
    execute_on = 'initial timestep_end'
  []
  [ghosted_entities]
    type = FeatureFloodCountAux
    variable = ghost_regions
    flood_counter = grain_tracker
    field_display = GHOSTED_ENTITIES
    execute_on = 'initial timestep_end'
  []
  [proc]
    type = ProcessorIDAux
    variable = proc
    execute_on = 'initial timestep_end'
  []
  [energy_density]
    # Total free energy for the tricrystal
    type = ADTotalFreeEnergy
    variable = free_energy
    kappa_names = "kappa_op kappa_op kappa_op"
    interfacial_vars = "gr0 gr1 gr2"
  []
[]

[Materials]
  [5DGaussian]
    # Material properties adding anisotropy to epsilon - also called kappa, m - also called mu, and L for epsilon model
    type =  SphericalGaussianMaterial
    outputs = exodus
    model_type = EPSILON # The model to use. Epsilon or Gamma model?
    grain_tracker = grain_tracker # Grain tracker UserObject
    var_name_base = gr # Name base of array of coupled order parameter variables
    mob_L_name = L # The name of the anisotropic L
    library_file_name = "minima_library_tricrystal" # Name of the file containing minima misorientations (minima library) data
    quaternion_file_name = "3grains_quaternions" # Name of the file containing orientation quaternions data
    base_energy = 1.5 # (J/m^2); Base value of energy from which gaussian is either added or substracted
    base_L  = 0.006 # (nm^3/eV.ns) ; Base value of L from which gaussian is either added or substracted
    library_misorientations_number = 3 # Number of minima misorientations in the minima library file to use
    op_num = 3 # Number of coupled order parameter variables
    grain_boundary_width = 40 # (nm); Grain boundary width
    sharpness = 20 # (dimensionless); Gaussian sharpness
    anisotropic_epsilon = true # If true, use anisotropic epsilon; otherwise use isotropic epsilon
    anisotropic_m = true # If true, use anisotropic m; otherwise use isotropic m
    anisotropic_L = true # If true, use anisotropic L; otherwise use isotropic L
    add_epsilon_gaussian = false # 'true' to add to base value; 'false' to substract from base value epsilon
    add_L_gaussian = false # 'true' to add to base value; 'false' to substract from base value L
    compute_final_gaussian_direction = false # If true, the final gaussian direction is computed using the orientation quaternions, else taken directly from the minima library file
    set_bulk_to_base_values = true # If true, the anisotropic material properties are at base values within the grains, else zero
  []
  [free_energy]
    # Free energy defined for three grains variables 'gr0 gr1 gr2'
    type = ADDerivativeParsedMaterial
    property_name = F
    material_property_names = 'mu gamma_asymm'
    coupled_variables = 'gr0 gr1 gr2'
    expression = 'mu * ((((gr0^4)/4) + ((gr1^4)/4) + ((gr2^4)/4)) -  (((gr0^2)/2) + ((gr1^2)/2) + ((gr2^2)/2)) + (gamma_asymm * (((gr0^2)*(gr1^2)) + ((gr0^2)*(gr2^2)) + ((gr1^2)*(gr2^2)))) + (1.0/4.0))'
  []
[]

[Postprocessors]
  [dt]
    # Outputs the current time step
    type = TimestepSize
  []
  [total_energy]
    # Total free energy (eV)
    type = ElementIntegralVariablePostprocessor
    variable = free_energy
    execute_on = 'initial timestep_end'
  []
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -snes_type'
  petsc_options_value = ' lu     vinewtonrsls'
  nl_max_its = 20
  l_max_its = 30
  l_tol = 1e-4
  nl_rel_tol = 1e-12
  nl_abs_tol = 1e-12
  end_time = 500000 # (ns)
  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 6
    iteration_window = 2
    dt = 10 # (ns)
    growth_factor = 1.1
    cutback_factor = 0.75
  []
[]

[Outputs]
  exodus = true
  execute_on = 'timestep_end'  
[]
