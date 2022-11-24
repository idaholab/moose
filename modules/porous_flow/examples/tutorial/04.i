# Darcy flow with heat advection and conduction, and elasticity
[Mesh]
  [annular]
    type = AnnularMeshGenerator
    nr = 10
    rmin = 1.0
    rmax = 10
    growth_r = 1.4
    nt = 4
    dmin = 0
    dmax = 90
  []
  [make3D]
    type = MeshExtruderGenerator
    extrusion_vector = '0 0 12'
    num_layers = 3
    bottom_sideset = 'bottom'
    top_sideset = 'top'
    input = annular
  []
  [shift_down]
    type = TransformGenerator
    transform = TRANSLATE
    vector_value = '0 0 -6'
    input = make3D
  []
  [aquifer]
    type = SubdomainBoundingBoxGenerator
    block_id = 1
    bottom_left = '0 0 -2'
    top_right = '10 10 2'
    input = shift_down
  []
  [injection_area]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x*x+y*y<1.01'
    included_subdomain_ids = 1
    new_sideset_name = 'injection_area'
    input = 'aquifer'
  []
  [rename]
    type = RenameBlockGenerator
    old_block = '0 1'
    new_block = 'caps aquifer'
    input = 'injection_area'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
  biot_coefficient = 1.0
[]

[Variables]
  [porepressure]
  []
  [temperature]
    initial_condition = 293
    scaling = 1E-8
  []
  [disp_x]
    scaling = 1E-10
  []
  [disp_y]
    scaling = 1E-10
  []
  [disp_z]
    scaling = 1E-10
  []
[]

[PorousFlowBasicTHM]
  porepressure = porepressure
  temperature = temperature
  coupling_type = ThermoHydroMechanical
  gravity = '0 0 0'
  fp = the_simple_fluid
  eigenstrain_names = thermal_contribution
  use_displaced_mesh = false
[]

[BCs]
  [constant_injection_porepressure]
    type = DirichletBC
    variable = porepressure
    value = 1E6
    boundary = injection_area
  []
  [constant_injection_temperature]
    type = DirichletBC
    variable = temperature
    value = 313
    boundary = injection_area
  []

  [roller_tmax]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = dmax
  []
  [roller_tmin]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = dmin
  []
  [roller_top_bottom]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'top bottom'
  []
  [cavity_pressure_x]
    type = Pressure
    boundary = injection_area
    variable = disp_x
    component = 0
    factor = 1E6
    use_displaced_mesh = false
  []
  [cavity_pressure_y]
    type = Pressure
    boundary = injection_area
    variable = disp_y
    component = 1
    factor = 1E6
    use_displaced_mesh = false
  []
[]

[AuxVariables]
  [stress_rr]
    family = MONOMIAL
    order = CONSTANT
  []
  [stress_pp]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [stress_rr]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = stress_rr
    scalar_type = RadialStress
    point1 = '0 0 0'
    point2 = '0 0 1'
  []
  [stress_pp]
    type = RankTwoScalarAux
    rank_two_tensor = stress
    variable = stress_pp
    scalar_type = HoopStress
    point1 = '0 0 0'
    point2 = '0 0 1'
  []
[]

[FluidProperties]
  [the_simple_fluid]
    type = SimpleFluidProperties
    bulk_modulus = 2E9
    viscosity = 1.0E-3
    density0 = 1000.0
    thermal_expansion = 0.0002
    cp = 4194
    cv = 4186
    porepressure_coefficient = 0
  []
[]

[Materials]
  [porosity]
    type = PorousFlowPorosity
    porosity_zero = 0.1
  []
  [biot_modulus]
    type = PorousFlowConstantBiotModulus
    solid_bulk_compliance = 2E-7
    fluid_bulk_modulus = 1E7
  []
  [permeability_aquifer]
    type = PorousFlowPermeabilityConst
    block = aquifer
    permeability = '1E-14 0 0   0 1E-14 0   0 0 1E-14'
  []
  [permeability_caps]
    type = PorousFlowPermeabilityConst
    block = caps
    permeability = '1E-15 0 0   0 1E-15 0   0 0 1E-16'
  []

  [thermal_expansion]
    type = PorousFlowConstantThermalExpansionCoefficient
    drained_coefficient = 0.003
    fluid_coefficient = 0.0002
  []
  [rock_internal_energy]
    type = PorousFlowMatrixInternalEnergy
    density = 2500.0
    specific_heat_capacity = 1200.0
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '10 0 0  0 10 0  0 0 10'
    block = 'caps aquifer'
  []

  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 5E9
    poissons_ratio = 0.0
  []
  [strain]
    type = ComputeSmallStrain
    eigenstrain_names = thermal_contribution
  []
  [thermal_contribution]
    type = ComputeThermalExpansionEigenstrain
    temperature = temperature
    thermal_expansion_coeff = 0.001 # this is the linear thermal expansion coefficient
    eigenstrain_name = thermal_contribution
    stress_free_temperature = 293
  []
  [stress]
    type = ComputeLinearElasticStress
  []
[]

[Preconditioning]
  active = basic
  [basic]
    type = SMP
    full = true
    petsc_options = '-ksp_diagonal_scale -ksp_diagonal_scale_fix'
    petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_shift_type -pc_asm_overlap'
    petsc_options_value = ' asm      lu           NONZERO                   2'
  []
  [preferred_but_might_not_be_installed]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
    petsc_options_value = ' lu       mumps'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  end_time = 1E6
  dt = 1E5
  nl_abs_tol = 1E-15
  nl_rel_tol = 1E-14
[]

[Outputs]
  exodus = true
[]
