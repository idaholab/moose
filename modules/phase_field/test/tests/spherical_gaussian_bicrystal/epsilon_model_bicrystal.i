#  Epsilon Model - Bicrystal

[Mesh]
  type = GeneratedMesh
   dim = 2
   nx = 80
   ny = 40
   xmin = 0
   xmax = 800
   ymin = 0
   ymax = 400
   elem_type = QUAD4
[]

[GlobalParams]
  op_num = 2
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
    variable = 'gr0 gr1'
    threshold = 0.2
    connecting_threshold = 0.08
    compute_var_to_feature_map = true
  []
[]

[ICs]
  [gr0_IC]
    type = BoundingBoxIC
     variable = gr0
     outside = 0.0
     inside = 1.0
     x1 = 400.0
     x2 = 800.0
     y1 = 0.0
     y2 = 400.0
  []
  [gr1_IC]
    type = BoundingBoxIC
     variable = gr1
     outside = 0.0
     inside = 1.0
     x1 = 0.0
     x2 = 400.0
     y1 = 0.0
     y2 = 400.0
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
    # Total free energy for the bicrystal
    type = ADTotalFreeEnergy
    f_name = F
    variable = free_energy
    kappa_names = "kappa_op kappa_op"
    interfacial_vars = "gr0 gr1"
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
    library_file_name = "minima_library_bicrystal" # Name of the file containing minima misorientations (minima library) data
    quaternion_file_name = "2grains_quaternions_Inclined_20degrees" # Name of the file containing orientation quaternions data
    base_energy = 1.5 # (J/m^2); Base value of energy from which gaussian is either added or substracted
    base_L  = 0.006 # (nm^3/eV.ns) ; Base value of L from which gaussian is either added or substracted
    library_misorientations_number = 1 # Number of minima misorientations in the minima library file to use
    op_num = 2 # Number of coupled order parameter variables
    grain_boundary_width = 40 # (nm); Grain boundary width
    sharpness = 20 # (dimensionless); Gaussian sharpness
    anisotropic_epsilon = true # If true, use anisotropic epsilon; otherwise use isotropic epsilon
    anisotropic_m = true # If true, use anisotropic m; otherwise use isotropic m
    anisotropic_L = true # If true, use anisotropic L; otherwise use isotropic L
    add_epsilon_gaussian = false # 'true' to add to base value; 'false' to substract from base value epsilon
    add_L_gaussian = false # 'true' to add to base value; 'false' to substract from base value L
    compute_final_gaussian_direction = true # If true, the final gaussian direction is computed using the orientation quaternions, else taken directly from the minima library file
    set_bulk_to_base_values = true # If true, the anisotropic material properties are at base values within the grains, else zero
  []
  [free_energy]
    # Free energy defined for two grains variables 'gr0 gr1'
    type = ADDerivativeParsedMaterial
    property_name = F
    material_property_names = 'mu gamma_asymm'
    coupled_variables = 'gr0 gr1'
    expression = 'mu * ( (((gr0^4)/4) + ((gr1^4)/4)) -  (((gr0^2)/2) + ((gr1^2)/2))  +  ( gamma_asymm * ( ((gr0^2)*(gr1^2)) ) )  + (1.0/4.0) )'
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
  [Grain_boundary_area]
    # Bicrystal grain boundary area (nm^2)
    type = GrainBoundaryArea
    grains_per_side = 2
  []
  [energy_eVpernm2]
    # Total free energy (eV/nm^2)
    type = ParsedPostprocessor
    expression = 'total_energy / Grain_boundary_area'
    pp_names = 'total_energy Grain_boundary_area'
    execute_on = 'initial timestep_end'
  []
  [energy_Jperm2]
    # Total free energy (J/m^2)
    type = ParsedPostprocessor
    expression = 'energy_eVpernm2 / 6.242'
    pp_names = 'energy_eVpernm2'
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
  num_steps = 3
  dt = 3 # (ns)
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = 'timestep_end'
[]
