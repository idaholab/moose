[Mesh]
  [annular]
    type = AnnularMeshGenerator
    nr = 40
    nt = 16
    rmin = 0.1
    rmax = 1
    dmin = 0.0
    dmax = 90
    growth_r = 1.1
  []
  [make3D]
    input = annular
    type = MeshExtruderGenerator
    bottom_sideset = bottom
    top_sideset = top
    extrusion_vector = '0 0 1'
    num_layers = 1
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  PorousFlowDictator = dictator
  biot_coefficient = 1.0
[]

[Variables]
  [disp_x]
  []
  [disp_y]
  []
  [disp_z]
  []
  [porepressure]
  []
  [temperature]
  []
[]

[BCs]
  # sideset 1 = outer
  # sideset 2 = cavity
  # sideset 3 = ymin
  # sideset 4 = xmin
  [plane_strain]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'top bottom'
  []
  [ymin]
    type = DirichletBC
    variable = disp_y
    value = 0
    boundary = dmin
  []
  [xmin]
    type = DirichletBC
    variable = disp_x
    value = 0
    boundary = dmax
  []

  [cavity_temperature]
    type = DirichletBC
    variable = temperature
    value = 1000
    boundary = rmin
  []
  [cavity_porepressure]
    type = DirichletBC
    variable = porepressure
    value = 1E6
    boundary = rmin
  []
  [cavity_zero_effective_stress_x]
    type = Pressure
    variable = disp_x
    function = 1E6
    boundary = rmin
    use_displaced_mesh = false
  []
  [cavity_zero_effective_stress_y]
    type = Pressure
    variable = disp_y
    function = 1E6
    boundary = rmin
    use_displaced_mesh = false
  []

  [outer_temperature]
    type = DirichletBC
    variable = temperature
    value = 0
    boundary = rmax
  []
  [outer_pressure]
    type = DirichletBC
    variable = porepressure
    value = 0
    boundary = rmax
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
    thermal_expansion = 0.0
    bulk_modulus = 1E12
    viscosity = 1.0E-3
    density0 = 1000.0
    cv = 1000.0
    cp = 1000.0
    porepressure_coefficient = 0.0
  []
[]

[PorousFlowBasicTHM]
  coupling_type = ThermoHydroMechanical
  multiply_by_density = false
  add_stress_aux = true
  porepressure = porepressure
  temperature = temperature
  eigenstrain_names = thermal_contribution
  gravity = '0 0 0'
  fp = the_simple_fluid
[]

[Materials]
  [elasticity_tensor]
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = 1E10
    poissons_ratio = 0.2
  []
  [strain]
    type = ComputeSmallStrain
    eigenstrain_names = thermal_contribution
  []
  [thermal_contribution]
    type = ComputeThermalExpansionEigenstrain
    temperature = temperature
    thermal_expansion_coeff = 1E-6
    eigenstrain_name = thermal_contribution
    stress_free_temperature = 0.0
  []
  [stress]
    type = ComputeLinearElasticStress
  []
  [porosity]
    type = PorousFlowPorosityConst # only the initial value of this is ever used
    porosity = 0.1
  []
  [biot_modulus]
    type = PorousFlowConstantBiotModulus
    solid_bulk_compliance = 1E-10
    fluid_bulk_modulus = 1E12
  []
  [permeability]
    type = PorousFlowPermeabilityConst
    permeability = '1E-12 0 0   0 1E-12 0   0 0 1E-12'
  []
  [thermal_expansion]
    type = PorousFlowConstantThermalExpansionCoefficient
    fluid_coefficient = 1E-6
    drained_coefficient = 1E-6
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '1E6 0 0  0 1E6 0  0 0 1E6'
  []
[]

[VectorPostprocessors]
  [P]
    type = LineValueSampler
    start_point = '0.1 0 0'
    end_point = '1.0 0 0'
    num_points = 10
    sort_by = x
    variable = porepressure
  []
  [T]
    type = LineValueSampler
    start_point = '0.1 0 0'
    end_point = '1.0 0 0'
    num_points = 10
    sort_by = x
    variable = temperature
  []
  [U]
    type = LineValueSampler
    start_point = '0.1 0 0'
    end_point = '1.0 0 0'
    num_points = 10
    sort_by = x
    variable = disp_x
  []
[]

[Preconditioning]
  [andy]
    type = SMP
    full = true
    petsc_options_iname = '-ksp_type -pc_type -sub_pc_type -snes_rtol'
    petsc_options_value = 'gmres      asm      lu           1E-8'
  []
[]

[Executioner]
  type = Steady
  solve_type = Newton
[]

[Outputs]
  file_base = free_outer
  execute_on = timestep_end
  csv = true
[]
