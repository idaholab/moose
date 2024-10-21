[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = true
[]

[Mesh]
  [left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.05
    xmax = -0.05
    ymin = -1
    ymax = 0
    nx = 4
    ny = 8
    elem_type = QUAD4
  []
  [left_block_sidesets]
    type = RenameBoundaryGenerator
    input = left_block
    old_boundary = '0 1 2 3'
    new_boundary = '10 11 12 13'
  []
  [left_block_id]
    type = SubdomainIDGenerator
    input = left_block_sidesets
    subdomain_id = 1
  []
  [right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 4
    ny = 8
    elem_type = QUAD4
  []
  [right_block_sidesets]
    type = RenameBoundaryGenerator
    input = right_block
    old_boundary = '0 1 2 3'
    new_boundary = '20 21 22 23'
  []
  [right_block_id]
    type = SubdomainIDGenerator
    input = right_block_sidesets
    subdomain_id = 2
  []

  [combined_mesh]
    type = MeshCollectionGenerator
    inputs = 'left_block_id right_block_id'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    strain = FINITE
    incremental = true
    add_variables = true
    block = '1 2'
  []
[]

[Functions]
  [horizontal_movement]
    type = PiecewiseLinear
    x ='0 0.5 2'
    y = '0 0.1 0.1'
  []
  [vertical_movement]
    type = PiecewiseLinear
    x ='0 0.5 2'
    y = '0.001 0.001 0.2'
  []
[]

[BCs]
  [push_left_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 13
    function = horizontal_movement
  []
  [fix_right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 21
    value = 0.0
  []
  [fix_right_y]
    type = DirichletBC
    variable = disp_y
    boundary = 21
    value = 0.0
  []
  [push_left_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 13
    function = vertical_movement
  []
[]

[Materials]
  [elasticity_tensor_left]
    type = ComputeIsotropicElasticityTensor
    block = 1
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [stress_left]
    type = ComputeFiniteStrainElasticStress
    block = 1
  []

  [elasticity_tensor_right]
    type = ComputeIsotropicElasticityTensor
    block = 2
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
  []
  [stress_right]
    type = ComputeFiniteStrainElasticStress
    block = 2
  []
[]

[Contact]
  [leftright]
    secondary = '11'
    primary = '23'
    formulation = mortar
    model = frictionless
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options = '-snes_converged_reason -ksp_converged_reason -snes_view'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu NONZERO 1e-10'

  dt = 0.2
  dtmin = 0.2
  end_time = 1.0

  l_max_its = 20

  nl_max_its = 8
  nl_rel_tol = 1e-6
  snesmf_reuse_base = false
[]

[Outputs]
  file_base = ./dm_contact_gmesh_out
  [comp]
    type = CSV
    show = 'contact normal_lm avg_disp_x avg_disp_y max_disp_x max_disp_y min_disp_x min_disp_y'
    execute_on = 'FINAL'
  []
[]

[Postprocessors]
  [contact]
    type = ContactDOFSetSize
    variable = leftright_normal_lm
    subdomain = leftright_secondary_subdomain
  []
  [normal_lm]
    type = ElementAverageValue
    variable = leftright_normal_lm
    block = leftright_secondary_subdomain
  []
  [avg_disp_x]
    type = ElementAverageValue
    variable = disp_x
    block = '1 2'
  []
  [avg_disp_y]
    type = ElementAverageValue
    variable = disp_y
    block = '1 2'
  []
  [max_disp_x]
    type = ElementExtremeValue
    variable = disp_x
    block = '1 2'
  []
  [max_disp_y]
    type = ElementExtremeValue
    variable = disp_y
    block = '1 2'
  []
  [min_disp_x]
    type = ElementExtremeValue
    variable = disp_x
    block = '1 2'
    value_type = min
  []
  [min_disp_y]
    type = ElementExtremeValue
    variable = disp_y
    block = '1 2'
    value_type = min
  []
[]
