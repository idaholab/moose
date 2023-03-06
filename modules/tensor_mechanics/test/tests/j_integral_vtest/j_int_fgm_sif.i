[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y disp_z'
  volumetric_locking_correction = true
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = multiple_blocks_bimaterial.e
  []
  partitioner = centroid
  centroid_partitioner_direction = z
[]

[AuxVariables]
  [SED]
    order = CONSTANT
    family = MONOMIAL
  []
  [resid_z]
  []
[]

[Functions]
  [rampConstantUp]
    type = PiecewiseLinear
    x = '0. 1.'
    y = '0. 1'
    scale_factor = -68.95 #MPa
  []
  [elastic_mod_material_der]
    type = ParsedFunction
    expression = 'if(y < 229, 0.0, if(y>279, 0, 20680*0460517019*exp(0.0460517019*(y-229))))'
  []
  [elastic_mod_material]
    type = ParsedFunction
    expression = 'if(y < 229, 20680, if(y>279, 206800, 20680*exp(0.0460517019*(y-229))))'
  []
[]

[DomainIntegral]
  integrals = 'JIntegral InteractionIntegralKI'
  boundary = 1001
  crack_direction_method = CurvedCrackFront
  crack_end_direction_method = CrackDirectionVector
  crack_direction_vector_end_1 = '0.0 1.0 0.0'
  crack_direction_vector_end_2 = '1.0 0.0 0.0'
  radius_inner = '12.5 25.0 100'
  radius_outer = '25.0 37.5 150.0'
  intersecting_boundary = '1 2'
  symmetry_plane = 2
  incremental = true

  functionally_graded_youngs_modulus = elastic_mod_material_mat

  youngs_modulus = 20680
  poissons_ratio = 0.3
  block = '1 2'
[]

[Modules/TensorMechanics/Master]
  [master]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress'
    block = '1 2'
  []
[]

[AuxKernels]
  [SED]
    type = MaterialRealAux
    variable = SED
    property = strain_energy_density
    execute_on = timestep_end
  []
[]

[BCs]
  [crack_y]
    type = DirichletBC
    variable = disp_z
    boundary = 6
    value = 0.0
  []
  [no_y]
    type = DirichletBC
    variable = disp_y
    boundary = 12
    value = 0.0
  []
  [no_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  []
  [Pressure]
    [Side1]
      boundary = 5
      function = rampConstantUp # BCs
    []
  []
[]

[Materials]
  [generic_materials]
    type = GenericFunctionMaterial
    prop_names = 'elastic_mod_material_mat elastic_mod_material_der_mat'
    prop_values = 'elastic_mod_material elastic_mod_material_der'
  []
  [elasticity_tensor]
    type = ComputeVariableIsotropicElasticityTensor
    youngs_modulus = elastic_mod_material_mat
    poissons_ratio = 0.3
    args = ''
  []
  [elastic_stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu       superlu_dist'


  line_search = 'none'

  l_max_its = 50
  nl_max_its = 20
  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-8
  l_tol = 1e-6

  start_time = 0.0
  dt = 1

  end_time = 1
  num_steps = 1
[]

[Postprocessors]
  [_dt]
    type = TimestepSize
  []
  [nl_its]
    type = NumNonlinearIterations
  []
  [lin_its]
    type = NumLinearIterations
  []
  [react_z]
    type = NodalSum
    variable = resid_z
    boundary = 5
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
  exodus = true
[]
