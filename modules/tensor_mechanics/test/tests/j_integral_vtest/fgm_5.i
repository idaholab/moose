[GlobalParams]
  order = FIRST
  family = LAGRANGE
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  [file]
    type = FileMeshGenerator
    file = 2d_paulino.e
  []
  # uniform_refine = 3
[]

[DomainIntegral]
  integrals = 'JIntegral InteractionIntegralKI'
  boundary = 1001
  radius_inner = '0.01 0.04 0.1 0.2'
  radius_outer = '0.01 0.04 0.1 0.2'
  crack_direction_method = CrackDirectionVector
  crack_direction_vector = '1 0 0' # is it +?
  2d = true
  axis_2d = 2
  incremental = true
  symmetry_plane = 1

  functionally_graded_material_crack = true
  space_dependent_youngs_modulus = elastic_mod_material
  youngs_modulus_derivative = elastic_mod_material_der

  youngs_modulus = 2e6
  poissons_ratio = 0.3
  block = '1'
[]

[Modules/TensorMechanics/Master]
  [master]
    strain = FINITE
    add_variables = true
    incremental = true
    generate_output = 'stress_xx stress_yy stress_zz vonmises_stress strain_xx strain_yy'
    decomposition_method = EigenSolution
    planar_formulation = PLANE_STRAIN
  []
[]

[AuxVariables]
  [resid_y]
  []
  [elastic_mod_material_der]
  []
  [elastic_mod_material]
  []
[]

[Functions]
  [parsed_load]
    type = ParsedFunction
    symbol_names = 'E1 E2 beta'
    symbol_values = '1e3 3e3 5'
    # expression = 'if(y < 229, 20680, if(y>279, 206800, 20680*exp(0.0460517019*(y-229))))'
    expression = '-1.0*((E1 + E2) / 2 + (E1 - E2)/2 * tanh(beta*(x+0.1)))'
  []
[]

[AuxKernels]
  [elastic_mod_material_der]
    type = ParsedAux
    # beta: 1/50 * ln (E2/E1). 50 refers to the area of transition: 279-229
    use_xyzt = true
    constant_names = 'E1 E2 beta'
    constant_expressions = '1e6 3e6 5'
    expression = '(E1 - E2) / 2 * beta * (1.0 - tanh(beta*(x+0.1)) * tanh(beta*(x+0.1)))'
    # expression = '1'
    variable = elastic_mod_material_der
  []
  [elastic_mod_material]
    type = ParsedAux
    # beta: 1/50 * ln (E2/E1). 50 refers to the area of transition: 279-229
    use_xyzt = true
    constant_names = 'E1 E2 beta'
    constant_expressions = '1e6 3e6 5'
    # expression = 'if(y < 229, 20680, if(y>279, 206800, 20680*exp(0.0460517019*(y-229))))'
    expression = '(E1 + E2) / 2 + (E1 - E2)/2 * tanh(beta*(x+0.1))'
    variable = elastic_mod_material
  []
[]

[BCs]
  [plane_1_x]
    type = DirichletBC
    variable = disp_x
    boundary = 10001
    value = 0.0
  []
  [plane_y]
    type = DirichletBC
    variable = disp_y
    boundary = '10005 6 1' #10001
    value = 0.0
  []
  [Pressure]
    [Side1]
      boundary = 4
      function = parsed_load # BCs
    []
  []
[]

[Materials]
  [youngs_modulus]
    type = DerivativeParsedMaterial
    property_name = youngs_modulus
    coupled_variables = elastic_mod_material
    expression = 'elastic_mod_material'
  []
  [elasticity_tensor]
    type = ComputeVariableIsotropicElasticityTensor
    youngs_modulus = youngs_modulus
    poissons_ratio = 0.3
    args = elastic_mod_material
  []
  [elastic_stress]
    type = ComputeFiniteStrainElasticStress
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
  dt = 1.0

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
    variable = resid_y
    boundary = '10001 10005'
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
  exodus = true
[]
