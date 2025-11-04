# Example: Injection into a uniform aquifer 10 x 10 x 5 km
# Drucker-Prager deformation
# Darcy flow
gravity = -9.81
solid_density = 2350
fluid_density = 1000
porosity0 = 0.1
[Mesh]
  type = GeneratedMesh
  dim = 3
  xmin = 0
  xmax = 1e4
  ymin = 0
  ymax = 1e4
  zmax = 0
  zmin = -5e3
  nx = 2
  ny = 2
  nz = 2
[]

[GlobalParams]
  PorousFlowDictator = dictator
  gravity = '0 0 ${gravity}'
  displacements = 'disp_x disp_y disp_z'
  strain_at_nearest_qp = true
[]

[FluidProperties]
  [simple_fluid]
    type = SimpleFluidProperties
    thermal_expansion = 0 # Not doing a thermal simulation
    bulk_modulus = 2E9
    density0 = ${fluid_density}
    viscosity = 5E-4
  []
[]

[PorousFlowFullySaturated]
  coupling_type = HydroMechanical
  porepressure = pp
  dictator_name = dictator
  fp = simple_fluid
  add_darcy_aux = false
  add_stress_aux = false
  stabilization = none
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [pp]
    scaling = 1E6
    [InitialCondition]
      type = FunctionIC
      function = ini_pp
    []
  []
[]

[Functions]
  [ini_stress]
    type = ParsedFunction
    expression = '-${gravity} * z * (${solid_density} - ${fluid_density}) * (1.0 - ${porosity0})'  # initial effective stress that should result from weight force
  []

  [ini_pp]
    type = ParsedFunction
    expression = '${gravity} * z * ${fluid_density} + 1E5'
  []
[]

[BCs]
  [p_top]
    type = FunctionDirichletBC
    variable = pp
    boundary = front
    function = ini_pp
  []

  [x_roller]
    type = DirichletBC
    variable = disp_x
    boundary = 'left right'
    value = 0
  []
  [y_roller]
    type = DirichletBC
    variable = disp_y
    boundary = 'top bottom'
    value = 0
  []
  [z_confined]
    type = DirichletBC
    variable = disp_z
    boundary = 'back front'
    value = 0
  []
[]

[UserObjects]
  [pls_total_outflow_mass]
    type = PorousFlowSumQuantity
  []

  # Cohesion
  [mc_coh]
    type = TensorMechanicsHardeningConstant
    value = 6.0E6
  []

  # Friction angle
  [mc_phi]
    type = TensorMechanicsHardeningConstant
    value = 35.0
    convert_to_radians = true
  []

  # Dilation angle
  [mc_psi]
    type = TensorMechanicsHardeningConstant
    value = 2
    convert_to_radians = true
  []

  # Drucker-Prager objects
  [dp]
    type = TensorMechanicsPlasticDruckerPragerHyperbolic
    mc_cohesion = mc_coh
    mc_friction_angle = mc_phi
    mc_dilation_angle = mc_psi
    yield_function_tolerance = 1E-3
    internal_constraint_tolerance = 1E-6
  []

  # Tensile strength
  [tens]
    type = TensorMechanicsHardeningConstant
    value = 3.0E6
  []

  # Compressive strength (cap on yield envelope)
  [compr_all]
    type = TensorMechanicsHardeningConstant
    value = 1E10
  []
[]

[Materials]
  [strain]
    type = ComputeIncrementalStrain
    eigenstrain_names = eigenstrain_all
  []

  [eigenstrain_all]
    type = ComputeEigenstrainFromInitialStress
    initial_stress = 'ini_stress 0 0  0 ini_stress 0  0 0 ini_stress'
    eigenstrain_name = eigenstrain_all
  []

  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    bulk_modulus = 3.3333E9
    shear_modulus = 2.5E9
  []

  [dp_mat]
    type = CappedDruckerPragerStressUpdate
    DP_model = dp
    tensile_strength = tens
    compressive_strength = compr_all
    smoothing_tol = 1E5
    yield_function_tol = 1E-3
    tip_smoother = 0
  []

  [stress]
    type = ComputeMultipleInelasticStress
    inelastic_models = dp_mat
  []

  # Permeability
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-13 0 0  0 1E-13 0  0 0 1E-13'
  []

  # Porosity
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = ${porosity0}
    biot_coefficient = 1.0
    solid_bulk = 1.0 # Required but irrelevant when biot_coefficient is unity
    mechanical = true
    fluid = true
  []

  # Density of saturated rock
  [density]
    type = PorousFlowTotalGravitationalDensityFullySaturatedFromPorosity
    rho_s = ${solid_density}
  []
[]

[DiracKernels]
  [pls]
    type = PorousFlowPolyLineSink
    variable = pp
    SumQuantityUO = pls_total_outflow_mass
    point_file = two_nodes.bh
    function_of = pressure
    fluid_phase = 0
    p_or_t_vals = '0 1E7'
    fluxes = '-1.59 -1.59'
  []
[]

[Preconditioning]
  [usual]
    type = SMP
    full = true
  []
[]

[Executioner]
  solve_type = Newton
  type = Transient
  dt = 1E6
  end_time = 1E6
  nl_rel_tol = 1E-7
[]

[Outputs]
  exodus = true
[]
