youngs_modulus = 10e11

[Mesh]
  type = MeshGeneratorMesh
  patch_update_strategy = iteration

  #
  # Binder
  #

  [binder_block]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 36
    xmin = 0
    xmax = 3.75e+2
    ymin = 0
    ymax = 3.00e+2
    boundary_id_offset = 800
    subdomain_ids = 800
    boundary_name_prefix = 'binder'
    elem_type = QUAD4
  []

  [binder_rm]
    type = SubdomainBoundingBoxGenerator
    input = binder_block
    block_id = 200
    bottom_left = '0.75e+2 0.75e+2 0'
    top_right = '3.75e+2 2.25e+2 0'
  []

  [binder_rm_nodeset]
    type = BoundingBoxNodeSetGenerator
    input = binder_rm
    new_boundary = 807
    bottom_left = '0.75e+2 0.75e+2 0'
    top_right = '7.5e+2 4.0e+2 0'
  []

  [binder_initial]
    type = BlockDeletionGenerator
    block = 200
    input = binder_rm_nodeset
    new_boundary = 815
  []

  [binder_inner_top]
    type = SideSetsFromBoundingBoxGenerator
    input = binder_initial
    included_boundaries = 815
    boundary_new = 812
    bottom_left = '0.7e+2 2.24e+2 0'
    top_right = '4e+2 2.5e+2 0'
  []

  [binder_inner_bottom]
    type = SideSetsFromBoundingBoxGenerator
    input = binder_inner_top
    included_boundaries = 815
    boundary_new = 810
    bottom_left = '0.7e+2 0.5e+2 0'
    top_right = '4e+2 0.8e+2 0'
  []

  [binder]
    type = SideSetsFromBoundingBoxGenerator
    input = binder_inner_bottom
    included_boundaries = 815
    boundary_new = 813
    bottom_left = '0.6e+2 0.6e+2 0'
    top_right = '0.78e+2 2.5e+2 0'
  []

  #
  # Crystal
  #

  [crystal_subdomain]
    type = GeneratedMeshGenerator
    nx = 13
    ny = 60
    dim = 2
    xmin = 0.75e+2
    xmax = 3.75e+2
    ymin = 0.75e+2
    ymax = 2.25e+2
    boundary_id_offset = 100
    subdomain_ids = 100
    boundary_name_prefix = 'sd1'
  []

  [crack1_subdomain]
    type = SubdomainBoundingBoxGenerator
    input = crystal_subdomain
    block_id = 510
    bottom_left = '1.25e+2 1.125e+2 0'
    top_right = '4.0e+2 1.15e+2 0'
  []

  [crack1_nodeset]
    type = BoundingBoxNodeSetGenerator
    input = crack1_subdomain
    new_boundary = crack1_ns
    bottom_left = '1.25e+2 1.125e+2 0'
    top_right = '4.0e+2 1.15e+2 0'
  []

  [crystal_c1]
    type = BlockDeletionGenerator
    block = 510
    input = crack1_nodeset
    new_boundary = 500
  []

  [crack2_subdomain]
    type = SubdomainBoundingBoxGenerator
    input = crystal_c1
    block_id = 610
    bottom_left = '1.25e+2 1.5e+2 0'
    top_right = '4.0e+2 1.525e+2 0'
  []

  [crack2_nodeset]
    type = BoundingBoxNodeSetGenerator
    input = crack2_subdomain
    new_boundary = crack2_ns
    bottom_left = '1.25e+2 1.5e+2 0'
    top_right = '4.0e+2 1.525e+2 0'
  []

  [crystal_c2]
    type = BlockDeletionGenerator
    block = 610
    input = crack2_nodeset
    new_boundary = 600
  []

  [crack3_subdomain]
    type = SubdomainBoundingBoxGenerator
    input = crystal_c2
    block_id = 710
    bottom_left = '1.25e+2 1.875e+2 0'
    top_right = '4.0e+2 1.9e+2 0'
  []

  [crack3_nodeset]
    type = BoundingBoxNodeSetGenerator
    input = crack3_subdomain
    new_boundary = crack3_ns
    bottom_left = '1.25e+2 1.875e+2 0'
    top_right = '4.0e+2 1.9e+2 0'
  []

  [crystal_c3]
    type = BlockDeletionGenerator
    block = 710
    input = crack3_nodeset
    new_boundary = 700
  []

  [crack1_lower]
    type = SideSetsFromBoundingBoxGenerator
    input = crystal_c3
    included_boundaries = 500
    boundary_new = 511
    bottom_left = '1e+2 1.1e+2 0'
    top_right = '4.0e+2 1.13e+2 0'
  []

  [crack1_upper]
    type = SideSetsFromBoundingBoxGenerator
    input = crack1_lower
    included_boundaries = 500
    boundary_new = 512
    bottom_left = '1e+2 1.14e+2 0'
    top_right = '4.0e+2 1.18e+2 0'
  []

  [crack2_lower]
    type = SideSetsFromBoundingBoxGenerator
    input = crack1_upper
    included_boundaries = 600
    boundary_new = 611
    bottom_left = '1e+2 1.3e+2 0'
    top_right = '4.0e+2 1.51e+2 0'
  []

  [crack2_upper]
    type = SideSetsFromBoundingBoxGenerator
    input = crack2_lower
    included_boundaries = 600
    boundary_new = 612
    bottom_left = '1e+2 1.52e+2 0'
    top_right = '4.0e+2 1.6e+2 0'
  []

  [crack3_lower]
    type = SideSetsFromBoundingBoxGenerator
    input = crack2_upper
    included_boundaries = 700
    boundary_new = 711
    bottom_left = '1e+2 1.8e+2 0'
    top_right = '4.0e+2 1.88e+2 0'
  []

  [crystal]
    type = SideSetsFromBoundingBoxGenerator
    input = crack3_lower
    included_boundaries = 700
    boundary_new = 712
    bottom_left = '1e+2 1.89e+2 0'
    top_right = '4.0e+2 1.95e+2 0'
  []

  [rve_final]
    type = CombinerGenerator
    inputs = 'binder crystal'
  []
[]
##################################################################################
[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [disp_x]
    order = FIRST
    family = LAGRANGE
  []
  [disp_y]
    order = FIRST
    family = LAGRANGE
  []
[]

###################################################################################
[AuxVariables]
  [temp]
    order = CONSTANT
    family = MONOMIAL
  []
  [irr]
    order = CONSTANT
    family = MONOMIAL
  []
  [initial_x]
    order = CONSTANT
    family = MONOMIAL
  []

  [strain_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [strain_y]
    order = CONSTANT
    family = MONOMIAL
  []
[]

###################################################################################
[Functions]
  [temp_def]
    type = ConstantFunction
    value = 800
  []
  [irr_def]
    type = ConstantFunction
    value = 9
  []
  [initial_x_def]
    type = ConstantFunction
    value = 825
  []
[]

###################################################################################
[Physics]

  [SolidMechanics]

    [QuasiStatic]
      [all]
        eigenstrain_names = 'thermal_strain irr_strain'
        add_variables = true
        generate_output = 'vonmises_stress'
        block = '100 800'
      []
    []
  []
[]

###################################################################################
[AuxKernels]

  [initial_x]
    type = FunctionAux
    variable = initial_x
    function = initial_x_def
    use_displaced_mesh = false
  []
  [tempfuncaux]
    type = FunctionAux
    variable = temp
    function = temp_def
    use_displaced_mesh = false
  []
  [irrfuncaux]
    type = FunctionAux
    variable = irr
    function = irr_def
    use_displaced_mesh = false
  []
[]

###################################################################################
[BCs]

  [right]
    type = DirichletBC
    boundary = 'binder_right sd1_right'
    variable = disp_x
    value = 0.
  []
[]

###################################################################################
[Contact]
  [bc_top]
    primary = 812
    secondary = 102
    formulation = mortar
    model = frictionless
    correct_edge_dropping = true
  []
  [bc_bottom]
    primary = 810
    secondary = 100
    formulation = mortar
    model = frictionless
    correct_edge_dropping = true
  []
  [bc_left]
    primary = 103
    secondary = 813
    formulation = mortar
    model = frictionless
    correct_edge_dropping = true
  []

  [crack_1]
    primary = 511
    secondary = 512
    formulation = mortar
    model = frictionless
    correct_edge_dropping = true
  []
  [crack_2]
    primary = 611
    secondary = 612
    formulation = mortar
    model = frictionless
    correct_edge_dropping = true
  []
  [crack_3]
    primary = 711
    secondary = 712
    formulation = mortar
    model = frictionless
    correct_edge_dropping = true
  []
[]

###################################################################################
[Materials]

  #
  # Binder properties - isotropic elasticity + 2 eigenstrains
  #

  [binder_elasticity_tensor]
    block = '800'
    type = ComputeIsotropicElasticityTensor
    youngs_modulus = ${youngs_modulus}
    poissons_ratio = 0.01
  []
  [binder_therm_prefactor]
    type = DerivativeParsedMaterial
    block = '800'
    coupled_variables = 'temp'
    property_name = binder_therm_prefactor
    constant_names = 'a T'
    constant_expressions = '1.3e-5 298' #'1.3e-5 298'
    expression = '(a*(temp-T))'
  []
  [binder_thermal_strain]
    type = ComputeVariableEigenstrain
    block = '800'
    eigen_base = '1 0 0 0 1 0 0 0 1'
    args = 'temp'
    prefactor = binder_therm_prefactor
    eigenstrain_name = thermal_strain
  []
  [binder_irr_prefactor]
    type = DerivativeParsedMaterial
    block = '800'
    coupled_variables = 'irr initial_x'
    property_name = binder_irr_prefactor
    constant_names = 'm'
    constant_expressions = '0'
    expression = '((m*irr)/100)'
  []
  [binder_irr_strain]
    type = ComputeVariableEigenstrain
    block = '800'
    eigen_base = '1 0 0 0 1 0 0 0 1'
    args = 'irr'
    prefactor = binder_irr_prefactor
    eigenstrain_name = irr_strain
  []

  #
  # Crystal properties - orthotropic elasticity + 2 eigenstrains
  #

  [elasticity_tensor]
    type = ComputeElasticityTensor
    block = '100'
    fill_method = orthotropic
    C_ijkl = '1.095e9 3.65e7 1.095e9 2.8568e8 9.549e6 9.549e6 0.01 0.01 0.3 0.3 0.01 0.01'
  []
  [therm_prefactor]
    type = DerivativeParsedMaterial
    block = '100'
    coupled_variables = 'temp'
    property_name = therm_prefactor
    constant_names = 'a T'
    constant_expressions = '1.3e-5 298' #'2.65e-5 298'
    expression = '(a*(temp-T))'
  []
  [thermal_strain]
    type = ComputeVariableEigenstrain
    block = '100'
    eigen_base = '-0.0577 0 0 0 1 0 0 0 1'
    args = 'temp'
    prefactor = therm_prefactor
    eigenstrain_name = thermal_strain
  []
  [irr_prefactor]
    type = DerivativeParsedMaterial
    block = '100'
    coupled_variables = 'irr initial_x'
    property_name = irr_prefactor
    constant_names = 'm'
    constant_expressions = '1'
    expression = '((m*irr)/100)'
  []
  [irr_strain]
    type = ComputeVariableEigenstrain
    block = '100'
    eigen_base = '-0.31 0 0 0 1 0 0 0 1'
    args = 'irr'
    prefactor = irr_prefactor
    eigenstrain_name = irr_strain
  []

  [stress]
    type = ComputeLinearElasticStress
    block = '100 800'
  []
[]

[Preconditioning]
  [prec1]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Steady
  solve_type = 'Newton'
  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu superlu_dist'
  nl_rel_tol = 1e-7
[]

[Outputs]
  exodus = true
[]
