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

[AuxVariables]
  [react_z]
  []
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

  functionally_graded_youngs_modulus = elastic_mod_material_mat
  functionally_graded_youngs_modulus_crack_dir_gradient = elastic_mod_material_der_mat

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

[Functions]
  [parsed_load]
    type = ParsedFunction
    symbol_names = 'E1 E2 beta'
    symbol_values = '1e3 3e3 5'
    expression = '-1.0*((E1 + E2) / 2 + (E1 - E2)/2 * tanh(beta*(x+0.1)))'
  []
  [elastic_mod_material_der]
    type = ParsedFunction
    symbol_names = 'E1 E2 beta'
    symbol_values = '1e6 3e6 5'
    expression = '(E1 - E2) / 2 * beta * (1.0 - tanh(beta*(x+0.1)) * tanh(beta*(x+0.1)))'
  []
  [elastic_mod_material]
    type = ParsedFunction
    symbol_names = 'E1 E2 beta'
    symbol_values = '1e6 3e6 5'
    expression = '(E1 + E2) / 2 + (E1 - E2)/2 * tanh(beta*(x+0.1))'
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
    variable = react_z
    boundary = '10005 6 1'
  []
[]

[Outputs]
  execute_on = 'timestep_end'
  csv = true
  exodus = true
[]
