[GlobalParams]
  order = SECOND
  family = LAGRANGE
  displacements = 'disp_x disp_y'
[]

[Problem]
  type = ReferenceResidualProblem
  reference_vector = 'ref'
  extra_tag_vectors = 'ref'
  material_coverage_check = false
  kernel_coverage_check = false
[]

[Mesh]
  coord_type = 'RZ'
  displacements = 'disp_x disp_y'
  add_subdomain_ids = '9999'
  [block_A]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 2
    ymin = 5
    ymax = 6
    nx = 1
    ny = 1
    elem_type = QUAD8
    boundary_id_offset = 0
    subdomain_ids = 1
    subdomain_name = A
    boundary_name_prefix = a
  []
  [block_B]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 1
    xmax = 2
    ymin = 0
    ymax = 5
    nx = 1
    ny = 1
    elem_type = QUAD8
    boundary_id_offset = 4
    subdomain_ids = 2
    subdomain_name = B
    boundary_name_prefix = b
  []

  [stitch]
    type = StitchMeshGenerator
    inputs = 'block_A block_B'
    stitch_boundaries_pairs = '0 6'
    clear_stitched_boundary_ids = false
    verbose_stitching = true
  []
  [outside_bdry]
    type = ParsedGenerateSideset
    input = stitch
    new_sideset_name = 'outside'
    combinatorial_geometry = 'x>1.95 & x < 2.05'
    included_subdomains = 2
    normal = '1 0 0'
  []
  [inside_bdry]
    type = ParsedGenerateSideset
    input = outside_bdry
    new_sideset_name = 'inside'
    combinatorial_geometry = 'x>.95 & x < 1.05'
    included_subdomains = 2
    normal = '-1 0 0'
  []
  final_generator = inside_bdry
  patch_size = 5000
  patch_update_strategy = auto#iteration
  partitioner = centroid
  centroid_partitioner_direction = y
[]

[Materials]
  # [block_A]
  #   type = GenericMaterialA
  #   block = A
  #   use_displaced_mesh = true
  # []
  [elasticity_tensor1]
    type = ComputeIsotropicElasticityTensor
    block = A
    youngs_modulus = 7.5e4
    poissons_ratio = -.999999999
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = A
  []

  # [block_9999]
  #   type = GenericMaterialB
  #   block = 9999
  #   use_displaced_mesh = true
  # []
  [elasticity_tensor9999]
    type =  ComputeIsotropicElasticityTensor
    block = 9999
    youngs_modulus = 7.5e4
    poissons_ratio = -.999999999
  []
  [stress9999]
    type = ComputeFiniteStrainElasticStress
    block = 9999
  []

  [thermal]
    type = HeatConductionMaterial
    # block = '2'
    thermal_conductivity = 16.0
    specific_heat = 330.0
  []
  [density]
    type = StrainAdjustedDensity
    # block = '2'
    strain_free_density = 6551.0
  []
  # [block_B]
  #   type = GenericMaterialB
  #   block = B
  #   # outputs = all
  #   # output_properties = 'adjustment_eigenstrain'
  # []
  [elasticity_tensor2]
    type =  ComputeIsotropicElasticityTensor
    block = B
    youngs_modulus = 2.0e11
    poissons_ratio = 0.345
  []
  [stress2]
    type = ComputeFiniteStrainElasticStress
    block = B
  []

  [stress_wrapped]
    type = ComputeLagrangianWrappedStress
    large_kinematics = true#false
    objective_rate = jaumann
  []
[]

[MeshModifiers]
  [switch_blocks]
    type = CoupledVarThresholdElementSubdomainModifier
    coupled_var = phi
    criterion_type = 'ABOVE'
    threshold = 0.5
    block = A
    subdomain_id = 9999
    #reinitialize_subdomains = '9999'
    execute_on = 'TIMESTEP_BEGIN'
    skip_restore_subdomain_changes = true
    restore_overridden_dofs = true
    execution_order_group = -5
  []
  # [switch_blocks2]
  #   type = CoupledVarThresholdElementSubdomainModifier
  #   coupled_var = phi2
  #   criterion_type = 'ABOVE'
  #   threshold = 0.5
  #   block = 9999
  #   subdomain_id = 2
  #   # reinitialize_subdomains = ''
  #   reinitialize_variables = ''
  #   execute_on = 'TIMESTEP_BEGIN'
  #   skip_restore_subdomain_changes = true
  #   restore_overridden_dofs = true
  #   #old_subdomain_reinitialized = false
  #       execution_order_group = -4
  # []
[]

[Variables]
  [temp]
  []
  [disp_x]
  []
  [disp_y]
  []
[]


[Functions]
  [pressure_func]
    type = ParsedFunction
    expression = 1#'if (t < 2 & t > 0, 1, 0)'
  []
[]

[Physics]
  [SolidMechanics]
    [QuasiStatic]
      [block_9999]
        add_variables = true
        block = 9999
        strain = FINITE
        formulation = TOTAL
        new_system = true
        # eigenstrain_names = 'adjustment_eigenstrain'
        generate_output = 'stress_xx
                           stress_yy
                           stress_xy
                           stress_zz
                           strain_xx
                           strain_yy
                           strain_xy
                           strain_zz'
        temperature = temp
        #incremental = true
        # decomposition_method = EigenSolution
      []
      [block_2]
        add_variables = true
        block = 'B'
        strain = FINITE
        formulation = TOTAL
        new_system = true
        # eigenstrain_names = 'adjustment_eigenstrain'
        generate_output = 'stress_xx
                           stress_yy
                           stress_xy
                           stress_zz
                           strain_xx
                           strain_yy
                           strain_xy
                           strain_zz'
        temperature = temp
        #incremental = true
        #decomposition_method = EigenSolution
      []
      [block_1]
        add_variables = true
        block = 'A'
        strain = FINITE
        formulation = TOTAL
        new_system = true
        generate_output = 'stress_xx
                           stress_yy
                           stress_xy
                           stress_zz
                           strain_xx
                           strain_yy
                           strain_xy
                           strain_zz'
        temperature = temp
        #incremental = true
        # decomposition_method = EigenSolution
      []
    []
  []
[]

[AuxVariables]
  [phi]
  []
  [phi2]
  []
[]



[BCs]
  [no_x_block_1]
    type = DirichletBC
    boundary = 3
    value = 0
    variable = disp_x
  []
  # [no_x_block_2]
  #   type = DirichletBC
  #   boundary = inside
  #   value = -.1
  #   variable = disp_x
  # []
  [no_y]
    type = DirichletBC
    boundary = 4
    value = 0
    variable = disp_y
  []

  [outer_pressure]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 5
    function = '-0.1*t'
  []
  # [outer_pressure2] # BAD, REMOVE
  #   type = FunctionDirichletBC
  #   variable = disp_x
  #   boundary = a_right
  #   function = '-0.1*t'
  # []

  # [Pressure]
  #   [outer_pressure]
  #   boundary = 5
  #   function = pressure_func
  #   factor = 101325
  #   []
  # []
[]



[Kernels]
  [heat]
    type = HeatConduction
    variable = temp
  []
  [heat_ie]
    type = HeatConductionTimeDerivative
    variable = temp
  []
[]

[AuxKernels]
  [switch_block]
    type = ParsedAux
    expression = "if(t>=1, 1, 0)"
    use_xyzt = true
    variable = phi
    use_displaced_mesh = false
  []
  [switch_block2]
    type = ParsedAux
    expression = "if(t>=1, 1, 0)"
    use_xyzt = true
    variable = phi2
    use_displaced_mesh = false
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
  solve_type = 'PJFNK'

  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -snes_type'
  petsc_options_value = 'lu       superlu_dist vinewtonrsls'
  automatic_scaling = true
  compute_scaling_once = false
  line_search = 'none'

  start_time = 0
  dt = 1
  end_time = 2#3#10
  verbose = true

  l_max_its = 100
  l_tol = 8e-3
  nl_max_its = 40
  nl_rel_tol = 1e-4
  nl_abs_tol = 1e-10

  [Quadrature]
    order = FIFTH
    side_order = SEVENTH
  []
[]

[Outputs]
  exodus = true
[]
