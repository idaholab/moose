# Tests the rigid body motion due to applied force of multiple particles.

# ***COPY AND PASTE THESE AS NEEDED***
# 'gr0 gr1 gr2 gr3 gr4 gr5 gr6 gr7 gr8 gr9 gr10 gr11 gr12 gr13 gr14 gr15 gr16 gr17 gr18 gr19'
# (gr0^2+gr1^2+gr2^2+gr3^2+gr4^2+gr5^2+gr6^2+gr7^2+gr8^2+gr9^2+gr10^2+gr11^2+gr12^2+gr13^2+gr14^2+gr15^2+gr16^2+gr17^2+gr18^2+gr19^2)
# (gr0^3+gr1^3+gr2^3+gr3^3+gr4^3+gr5^3+gr6^3+gr7^3+gr8^3+gr9^3+gr10^3+gr11^3+gr12^3+gr13^3+gr14^3+gr15^3+gr16^3+gr17^3+gr18^3+gr19^3)

[GlobalParams]
  op_num = 4
  var_name_base = gr
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 15
  ny = 15
  xmin = 0
  xmax = 600
  ymin = 0
  ymax = 600
  elem_type = QUAD4
  uniform_refine = 1
[]

[Variables]
  [./c]
  [../]
  [./w]
  [../]
  [./PolycrystalVariables] # Automatically creates order parameter variables
  [../]
[]

[AuxVariables]
  [./bnds]
  [../]
  [./force]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./free_energy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./unique_grains]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./var_indices]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./centroids]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[Functions]
  [./load_x]
    # Defines the force on the grains in the x-direction
    type = ParsedFunction
    expression = 0.005*cos(x*pi/600)
  [../]
  [./load_y]
    # Defines the force on the grains in the y-direction
    type = ConstantFunction
    value = 0.002
  [../]
[]

[Kernels]
  [./RigidBodyMultiKernel]
    # Creates all of the necessary Allen Cahn kernels automatically
    c = c
    f_name = f_loc
    mob_name = L
    kappa_name = kappa_gr
    grain_force = grain_force
    grain_volumes = grain_volumes
    grain_tracker_object = grain_center
  [../]
  # Cahn Hilliard kernels
  [./dt_w]
    type = CoupledTimeDerivative
    variable = w
    v = c
  [../]
  [./CH_wres]
    type = SplitCHWRes
    variable = w
    mob_name = M
  [../]
  [./CH_Parsed]
    type = SplitCHParsed
    variable = c
    f_name = f_loc
    w = w
    kappa_name = kappa_c
    coupled_variables = 'gr0 gr1 gr2 gr3' # Must be changed as op_num changes. Copy/paste from line 4
  [../]
  [./CH_RBM]
    type = MultiGrainRigidBodyMotion
    variable = w
    c = c
    v = 'gr0 gr1 gr2 gr3'
    grain_force = grain_force
    grain_volumes = grain_volumes
    grain_tracker_object = grain_center
  [../]
[]

[AuxKernels]
  [./force_x]
    type = FunctionAux
    variable = force
    function = load_x
  [../]
  [./force_y]
    type = FunctionAux
    variable = force
    function = load_y
  [../]
  [./energy_density]
    type = TotalFreeEnergy
    variable = free_energy
    f_name = f_loc
    kappa_names = kappa_c
    interfacial_vars = c
  [../]
  [./bnds]
    type = BndsCalcAux
    variable = bnds
  [../]
[]

[BCs]
  [./bcs]
    #zero flux BC
    type = NeumannBC
    value = 0
    variable = c
    boundary = '0 1 2 3'
  [../]
[]

[Materials]
  [./constants]
    type = GenericConstantMaterial
    prop_names = 'kappa_gr kappa_c M L'
    prop_values = '250 4000 4.5 60'
  [../]
  [./free_energy]
    type = DerivativeParsedMaterial
    property_name = f_loc
    constant_names = 'A B'
    constant_expressions = '450 1.5'
    coupled_variables = 'c gr0 gr1 gr2 gr3' #Must be changed as op_num changes. Copy/paste from line 4
    expression = 'A*c^2*(1-c)^2+B*(c^2+6*(1-c)*(gr0^2+gr1^2+gr2^2+gr3^2)
                -4*(2-c)*(gr0^3+gr1^3+gr2^3+gr3^3)
                +3*(gr0^2+gr1^2+gr2^2+gr3^2)^2)'
                                 #Copy/paste from lines 5-6
    derivative_order = 2
  [../]
  [./force_density]
    type = ExternalForceDensityMaterial
    c = c
    k = 10.0
    force_x = load_x
    force_y = load_y
  [../]
[]

[Postprocessors]
  [./total_energy]
    type = ElementIntegralVariablePostprocessor
    variable = free_energy
    execute_on = 'initial timestep_end'
  [../]
[]

[VectorPostprocessors]
  [./forces]
    type = GrainForcesPostprocessor
    grain_force = grain_force
  [../]
  [./grain_volumes]
    type = FeatureVolumeVectorPostprocessor
    flood_counter = grain_center
    execute_on = 'initial timestep_begin'
  [../]
[]

[UserObjects]
  [./grain_center]
    type = GrainTracker
    outputs = none
    compute_var_to_feature_map = true
    execute_on = 'initial timestep_begin'
  [../]
  [./grain_force]
    type = ComputeExternalGrainForceAndTorque
    grain_data = grain_center
    c = c
    etas = 'gr0 gr1 gr2 gr3'
    force_density = force_density_ext
    execute_on = 'linear nonlinear'
  [../]
[]

[Preconditioning]
  [./coupled]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  scheme = bdf2
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -ksp_gmres_restart -sub_ksp_type
                         -sub_pc_type -pc_asm_overlap'
  petsc_options_value = 'asm      31                  preonly
                         ilu          2'
  l_tol = 1e-05
  nl_max_its = 30
  l_max_its = 30
  nl_rel_tol = 1e-07
  nl_abs_tol = 1e-09
  start_time = 0.0
  end_time = 4
  dt = 0.05
[]

[Outputs]
  exodus = true
  perf_graph = true
  [./display]
    type = Console
    max_rows = 12
  [../]
[]

[ICs]
  [./concentration_IC]
    type = SpecifiedSmoothCircleIC
    x_positions = '150 450 150 450'
    y_positions = '150 150 450 450'
    z_positions = '0   0   0   0'
    radii =       '120 120 120 120'
    variable = c
    invalue = 1.0
    outvalue = 0.0
    int_width = 25
  [../]
  [./gr0_IC]
    type = SmoothCircleIC
    variable = gr0
    x1 = 150
    y1 = 150
    radius = 120
    invalue = 1.0
    outvalue = 0.0
    int_width = 25
  [../]
  [./gr1_IC]
    type = SmoothCircleIC
    variable = gr1
    x1 = 450
    y1 = 150
    radius = 120
    invalue = 1.0
    outvalue = 0.0
    int_width = 25
  [../]
  [./gr2_IC]
    type = SmoothCircleIC
    variable = gr2
    x1 = 150
    y1 = 450
    radius = 120
    invalue = 1.0
    outvalue = 0.0
    int_width = 25
  [../]
  [./gr3_IC]
    type = SmoothCircleIC
    variable = gr3
    x1 = 450
    y1 = 450
    radius = 120
    invalue = 1.0
    outvalue = 0.0
    int_width = 25
  [../]
[]
