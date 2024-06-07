
# ############################### Tricrystal - EPSILON MODEL - EPSandMandLanisoGAUSS ##########################

[Mesh]

  [Base_mesh]
  #  Build a square mesh (nm)
    type = GeneratedMeshGenerator
    dim = 2
    nx = 80
    ny = 40
    nz = 1
    xmin = 0
    xmax = 800
    ymin = 0
    ymax = 400
    zmin = 0
    zmax = 1
    elem_type = QUAD4
  []

[]


[GlobalParams]
  op_num = 3 # Number of order parameters use
  var_name_base = gr # Base name of grains
  order = CONSTANT
  family = MONOMIAL
[]


[Variables]
  # Variable block, where all variables in the simulation are declared

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
     theta2 =  106.5
     junction = '165 200 0'
  []

  [InitialCondition_gr2]
     type = TricrystalTripleJunctionIC
     op_index = 3
     variable = gr2
     theta1 = 140
     theta2 =  106.5
     junction = '165 200 0'
  []

[]


[BCs]
  #  Default Neumann

  # [Periodic]
  #   [top_bottom]
  #     auto_direction = 'y z' # Makes problem periodic in the x and y directions
  #   []
  # []

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

  [EPSILONmodelKernel]
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
    #  Total free energy for the bicrystal
    type = TotalFreeEnergy
    variable = free_energy
    kappa_names = "kappa_op kappa_op kappa_op"
    interfacial_vars = "gr0 gr1 gr2"
  []

[]


[Materials]

  [5DGaussian]
    # Material properties adding anisotropy to ε [or also called k], m and L.

    # Minima Library file: Normalized_Axis_X  Normalized_Axis_Y Normalized_Axis_Z  Angle(Radian) GB_Normal_X  GB_Normal_Y  GB_Normal_Z  Minimun_GB_energy(J/m^2) Minimun_L(nm^3/eV.ns)
    # Grains orienttation quaternions file: qw qx qy qz


    type = EPSandMandLanisoGAUSS
    outputs = exodus

    eps_name = eps
    epsenergy_name = kappa_op

    depsdx_name = depsdx
    depsdy_name = depsdy
    depsdz_name = depsdz
    depsdx_EN_name = depsdx_EN
    depsdy_EN_name = depsdy_EN
    depsdz_EN_name = depsdz_EN

    VwlibR_name = VwlibR
    VxlibR_name = VxlibR
    VylibR_name = VylibR
    VzlibR_name = VzlibR
    Vx_name = Vx
    Vy_name = Vy
    Vz_name = Vz

    dmdx_name = dmdx
    dmdy_name = dmdy
    dmdz_name = dmdz
    dmdx_EN_name = dmdx_EN
    dmdy_EN_name = dmdy_EN
    dmdz_EN_name = dmdz_EN
    dmdxplus_name = dmdxplus
    dmdyplus_name = dmdyplus
    dmdzplus_name = dmdzplus
    dmdxplus_EN_name = dmdxplus_EN
    dmdyplus_EN_name = dmdyplus_EN
    dmdzplus_EN_name = dmdzplus_EN

    depsdxplus_name = depsdxplus
    depsdyplus_name = depsdyplus
    depsdzplus_name = depsdzplus
    depsdxplus_EN_name = depsdxplus_EN
    depsdyplus_EN_name = depsdyplus_EN
    depsdzplus_EN_name = depsdzplus_EN

    S_switch_name = switch

    epsCalc_name = epsCalc
    mCalc_name = mCalc

    m_name = m
    m_energy_name = mu

    L_name = L_AD
    L_energy_name = L

    sigma_name = sigma
    sigmaORIUNIT_name = sigmaORIUNIT

    qwg_name = qwg
    qxg_name = qxg
    qyg_name = qyg
    qzg_name = qzg

    epsbar_name = epsbar
    epsmin2grains_name = epsmin2grains

    TotGauss_name = TotGauss

    grain_tracker = grain_tracker

    var_name_base = gr

    Library_file_name = "Minima_Library_TRI_EPSandMandLGAUSS" # Name given to the minima libray file

    Quaternion_file_name = "3grains_Quaternions" # Name given to the grains orientation quaternions file

    gamma = 1.5 # MAINTAIN AT 1.5

    sigmaBASE = 1.5 # (J/m^2); Base value of energy from  which gaussian is substracted or added

    L_BASE  = 0.0001 # (nm^3/eV.ns) ; Base value of L from which gaussian is substracted or added

    alphaswitch = 3 # Range of variation for switch 1

    betaswitch = 3 # Range of variation for switch 2

    libnum = 3 # Number of lines in minima libray file

    op_num = 3 # Number of lines in the grains orientation quaternions file

    lgb = 30 # (nm);  Grain boundary width

    sharpness = 20 # Width of gaussian

    amplitudeScale = 1 # MAINTAIN AT 1; Value to sacle the units.

    Gaussian_Tolerance = 1e-30 # Used to control gaussian effect; 1e-30 is enough

    ADDGaussian = false # 'true' to add to base value; 'false' to substract from base value

    ADDGaussianL = true # 'true' to add to base value; 'false' to substract from base value [L]

    BoundaryNormal = 5 # If = 0,the boundary normal in the simulation reference frame is computed using the orientations quaternions, else it is taken directly from the minima library file
  []

  [gamma_asymm]
  # MAINTAIN γ AT 1.5 --- γ is constant in Epsiilon model
    type = GenericConstantMaterial
    prop_names = 'gamma_asymm'
    prop_values = '1.5'
  []

  [free_energy]
  # Free energy defined for thre grains variables 'gr0 gr1 gr2'
    type = DerivativeParsedMaterial
    property_name = F
    material_property_names = 'mu'
    constant_names =       'phi  omega  gamma_asymm'
    constant_expressions = ' 1    1     1.5'
    coupled_variables = 'gr0 gr1 gr2'
    expression = 'mu * ( (phi * (((gr0^4)/4) + ((gr1^4)/4) + ((gr2^4)/4)) )  - (omega * (((gr0^2)/2) + ((gr1^2)/2) + ((gr2^2)/2)))
                +  ( gamma_asymm * ( ((gr0^2)*(gr1^2))+((gr0^2)*(gr2^2))+((gr1^2)*(gr2^2)) ) ) + (1.0/4.0) )'
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


[Preconditioning]

  [SMP]
    type = SMP
    full = true
  []

[]


[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart  -snes_type'
  petsc_options_value = ' hypre    boomeramg      31                  vinewtonrsls'

  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-5
  l_max_its = 30

  end_time = 400000 # (ns)

  [TimeStepper]
    type = IterationAdaptiveDT
    optimal_iterations = 6
    iteration_window = 2
    dt = 1 # (ns)
    growth_factor = 1.1
    cutback_factor = 0.75
  []

[]

[Debug]
  show_var_residual_norms = true
[]

[Outputs]
  exodus = true
[]
