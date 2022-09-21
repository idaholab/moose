# A version of fixed_outer.i that uses the RZ cylindrical coordinate system
[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40 # this is the r direction
  ny = 1 # this is the height direction
  xmin = 0.1
  xmax = 1
  bias_x = 1.1
  ymin = 0.0
  ymax = 1.0
[]

[Problem]
  coord_type = RZ
[]

[GlobalParams]
  displacements = 'disp_r disp_z'
  PorousFlowDictator = dictator
  biot_coefficient = 1.0
[]

[Variables]
  [disp_r]
  []
  [disp_z]
  []
  [porepressure]
  []
  [temperature]
  []
[]

[BCs]
  [plane_strain]
    type = DirichletBC
    variable = disp_z
    value = 0
    boundary = 'top bottom'
  []

  [cavity_temperature]
    type = DirichletBC
    variable = temperature
    value = 1000
    boundary = left
  []
  [cavity_porepressure]
    type = DirichletBC
    variable = porepressure
    value = 1E6
    boundary = left
  []
  [cavity_zero_effective_stress_x]
    type = Pressure
    variable = disp_r
    function = 1E6
    boundary = left
    use_displaced_mesh = false
  []

  [outer_temperature]
    type = DirichletBC
    variable = temperature
    value = 0
    boundary = right
  []
  [outer_pressure]
    type = DirichletBC
    variable = porepressure
    value = 0
    boundary = right
  []
  [fixed_outer_disp]
    type = DirichletBC
    variable = disp_r
    value = 0
    boundary = right
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
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_rr
    index_i = 0
    index_j = 0
  []
  [stress_pp] # hoop stress
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_pp
    index_i = 2
    index_j = 2
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
    type = ComputeAxisymmetricRZSmallStrain
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
    permeability = '1E-12 0 0   0 1E-12 0   0 0 1E-12' # note this is ordered: rr, zz, angle-angle
  []
  [thermal_expansion]
    type = PorousFlowConstantThermalExpansionCoefficient
    fluid_coefficient = 1E-6
    drained_coefficient = 1E-6
  []
  [thermal_conductivity]
    type = PorousFlowThermalConductivityIdeal
    dry_thermal_conductivity = '1E6 0 0  0 1E6 0  0 0 1E6' # note this is ordered: rr, zz, angle-angle
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
    variable = disp_r
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
  file_base = fixed_outer_rz
  execute_on = timestep_end
  csv = true
[]
